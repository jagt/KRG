#include "EntityMap.h"
#include "EntityActivationContext.h"
#include "Engine/Core/Entity/Entity.h"
#include "System/Resource/ResourceSystem.h"
#include "System/Core/Profiling/Profiling.h"
#include "Engine/Core/Entity/EntityLoadingContext.h"

//-------------------------------------------------------------------------

namespace KRG::EntityModel
{
    EntityMap::EntityMap()
        : m_entityUpdateEventBindingID( Entity::OnEntityInternalStateUpdated().Bind( [this] ( Entity* pEntity ) { OnEntityStateUpdated( pEntity ); } ) )
        , m_isTransientMap( true )
    {}

    EntityMap::EntityMap( ResourceID mapResourceID )
        : m_pMapDesc( mapResourceID )
        , m_entityUpdateEventBindingID( Entity::OnEntityInternalStateUpdated().Bind( [this] ( Entity* pEntity ) { OnEntityStateUpdated( pEntity ); } ) )
    {}

    EntityMap::EntityMap( EntityMap const& map )
        : m_entityUpdateEventBindingID( Entity::OnEntityInternalStateUpdated().Bind( [this] ( Entity* pEntity ) { OnEntityStateUpdated( pEntity ); } ) )
    {
        operator=( map );
    }

    EntityMap::EntityMap( EntityMap&& map )
        : m_entityUpdateEventBindingID( Entity::OnEntityInternalStateUpdated().Bind( [this] ( Entity* pEntity ) { OnEntityStateUpdated( pEntity ); } ) )
    {
        operator=( eastl::move( map ) );
    }

    EntityMap::~EntityMap()
    {
        KRG_ASSERT( IsUnloaded() && !m_isMapInstantiated );
        KRG_ASSERT( m_entities.empty() && m_entityIDLookupMap.empty() );
        KRG_ASSERT( m_entitiesToAdd.empty() && m_entitiesToRemove.empty() );

        #if KRG_DEVELOPMENT_TOOLS
        KRG_ASSERT( m_entitiesToHotReload.empty() );
        KRG_ASSERT( m_editedEntities.empty() );
        #endif

        Entity::OnEntityInternalStateUpdated().Unbind( m_entityUpdateEventBindingID );
        DestroyAllEntities();
    }

    //-------------------------------------------------------------------------

    bool EntityMap::CreateDescriptor( TypeSystem::TypeRegistry const& typeRegistry, EntityCollectionDescriptor& outCollectionDesc ) const
    {
        KRG_ASSERT( IsLoaded() || IsActivated() );
        KRG_ASSERT( m_entitiesToAdd.empty() && m_entitiesToRemove.empty() );

        #if KRG_DEVELOPMENT_TOOLS
        KRG_ASSERT( m_entitiesToHotReload.empty() );
        #endif

        TVector<StringID> entityNameList;
        outCollectionDesc.Clear();

        for ( auto pEntity : GetEntities() )
        {
            // Check for unique names
            if ( VectorContains( entityNameList, pEntity->GetName() ) )
            {
                KRG_LOG_ERROR( "Entity Model", "Failed to create entity collection descriptor, duplicate entity name found: %s", pEntity->GetName().c_str() );
                return false;
            }
            else
            {
                entityNameList.emplace_back( pEntity->GetName() );
            }

            //-------------------------------------------------------------------------

            EntityDescriptor entityDesc;
            if ( !pEntity->CreateDescriptor( typeRegistry, entityDesc ) )
            {
                return false;
            }
            outCollectionDesc.AddEntity( entityDesc );
        }

        outCollectionDesc.GenerateSpatialAttachmentInfo();

        return true;
    }

    //-------------------------------------------------------------------------

    EntityMap& EntityMap::operator=( EntityMap const& map )
    {
        // Only allow copy constructor for unloaded maps
        KRG_ASSERT( m_status == Status::Unloaded && map.m_status == Status::Unloaded );

        #if KRG_DEVELOPMENT_TOOLS
        KRG_ASSERT( map.m_entitiesToHotReload.empty() );
        #endif

        m_pMapDesc = map.m_pMapDesc;
        const_cast<bool&>( m_isTransientMap ) = map.m_isTransientMap;
        return *this;
    }

    EntityMap& EntityMap::operator=( EntityMap&& map )
    {
        #if KRG_DEVELOPMENT_TOOLS
        KRG_ASSERT( map.m_entitiesToHotReload.empty() );
        #endif

        m_ID = map.m_ID;
        m_entities.swap( map.m_entities );
        m_entityIDLookupMap.swap( map.m_entityIDLookupMap );
        m_pMapDesc = eastl::move( map.m_pMapDesc );
        m_entitiesCurrentlyLoading = eastl::move( map.m_entitiesCurrentlyLoading );
        m_status = map.m_status;
        m_isUnloadRequested = map.m_isUnloadRequested;
        m_isMapInstantiated = map.m_isMapInstantiated;
        const_cast<bool&>( m_isTransientMap ) = map.m_isTransientMap;

        // Clear source map
        map.m_ID.Clear();
        map.m_status = Status::Unloaded;
        map.m_isMapInstantiated = false;
        map.m_isUnloadRequested = false;
        return *this;
    }

    //-------------------------------------------------------------------------

    void EntityMap::AddEntityToLookupMaps( Entity* pEntity )
    {
        KRG_ASSERT( pEntity != nullptr && !pEntity->IsActivated() );
        KRG_ASSERT( !ContainsEntity( pEntity ) );

        m_entityIDLookupMap.insert( TPair<EntityID, Entity*>( pEntity->m_ID, pEntity ) );

        //-------------------------------------------------------------------------

        #if KRG_DEVELOPMENT_TOOLS
        pEntity->m_name = GenerateUniqueEntityName( pEntity->m_name );
        m_entityNameLookupMap.insert( TPair<StringID, Entity*>( pEntity->m_name, pEntity ) );
        #endif
    }

    void EntityMap::RemoveEntityFromLookupMaps( Entity* pEntity )
    {
        KRG_ASSERT( pEntity != nullptr && !pEntity->IsActivated() );

        //-------------------------------------------------------------------------

        auto IDLookupIter = m_entityIDLookupMap.find( pEntity->m_ID );
        KRG_ASSERT( IDLookupIter != m_entityIDLookupMap.end() );
        m_entityIDLookupMap.erase( IDLookupIter );

        //-------------------------------------------------------------------------

        #if KRG_DEVELOPMENT_TOOLS
        auto nameLookupIter = m_entityNameLookupMap.find( pEntity->m_name );
        KRG_ASSERT( nameLookupIter != m_entityNameLookupMap.end() );
        m_entityNameLookupMap.erase( nameLookupIter );
        #endif
    }

    #if KRG_DEVELOPMENT_TOOLS
    void EntityMap::RenameEntity( Entity* pEntity, StringID newNameID )
    {
        KRG_ASSERT( pEntity != nullptr && pEntity->m_mapID == m_ID );

        // Remove from lookup map
        auto nameLookupIter = m_entityNameLookupMap.find( pEntity->m_name );
        KRG_ASSERT( nameLookupIter != m_entityNameLookupMap.end() );
        m_entityNameLookupMap.erase( nameLookupIter );

        // Rename
        pEntity->m_name = GenerateUniqueEntityName( newNameID );

        // Add to lookup map
        m_entityNameLookupMap.insert( TPair<StringID, Entity*>( pEntity->m_name, pEntity ) );
    }
    #endif

    //-------------------------------------------------------------------------

    void EntityMap::AddEntities( TVector<Entity*> const& entities, Transform const& offsetTransform )
    {
        Threading::RecursiveScopeLock lock( m_mutex );

        m_entityIDLookupMap.reserve( m_entityIDLookupMap.size() + entities.size() );

        #if KRG_DEVELOPMENT_TOOLS
        m_entityNameLookupMap.reserve( m_entityNameLookupMap.size() + entities.size() );
        #endif

        //-------------------------------------------------------------------------

        bool const applyOffset = !offsetTransform.IsIdentity();
        for ( auto& pEntity : entities )
        {
            // Shift entity by the specified offset
            if ( applyOffset && pEntity->IsSpatialEntity() )
            {
                pEntity->SetWorldTransform( pEntity->GetWorldTransform() * offsetTransform );
            }

            AddEntity( pEntity );
        }
    }

    void EntityMap::AddEntity( Entity* pEntity )
    {
        // Ensure that the entity to add, is not already part of a collection and that it's deactivated
        KRG_ASSERT( pEntity != nullptr && !pEntity->IsAddedToMap() && !pEntity->HasRequestedComponentLoad() );
        KRG_ASSERT( !VectorContains( m_entitiesToAdd, pEntity ) );

        #if KRG_DEVELOPMENT_TOOLS
        if ( pEntity->HasSpatialParent() )
        {
            KRG_ASSERT( ContainsEntity( pEntity->GetSpatialParent() ) );
        }
        #endif

        //-------------------------------------------------------------------------

        Threading::RecursiveScopeLock lock( m_mutex );

        pEntity->m_mapID = m_ID;
        AddEntityToLookupMaps( pEntity );
        m_entitiesToAdd.emplace_back( pEntity );
    }

    Entity* EntityMap::RemoveEntity( EntityID entityID )
    {
        Entity* pEntityToRemove = nullptr;

        // Lock the map
        Threading::RecursiveScopeLock lock( m_mutex );

        // Check if the entity is in the add queue, if so just cancel the request
        int32 const entityIdx = VectorFindIndex( m_entitiesToAdd, entityID, [] ( Entity* pEntity, EntityID entityID ) { return pEntity->GetID() == entityID; } );
        if ( entityIdx != InvalidIndex )
        {
            pEntityToRemove = m_entitiesToAdd[entityIdx];
            m_entitiesToAdd.erase_unsorted( m_entitiesToAdd.begin() + entityIdx );
        }
        else // Queue removal
        {
            if ( m_isMapInstantiated )
            {
                pEntityToRemove = FindEntity( entityID );
                KRG_ASSERT( pEntityToRemove != nullptr );
                m_entitiesToRemove.emplace_back( RemovalRequest( pEntityToRemove, false ) );
            }
            else // Unknown entity
            {
                KRG_UNREACHABLE_CODE();
            }
        }

        return pEntityToRemove;
    }

    void EntityMap::DestroyEntity( EntityID entityID )
    {
        Entity* pEntityToDestroy = nullptr;

        // Lock the map
        Threading::RecursiveScopeLock lock( m_mutex );

        // Check if the entity is in the add queue, if so just cancel the request
        int32 const entityIdx = VectorFindIndex( m_entitiesToAdd, entityID, [] ( Entity* pEntity, EntityID entityID ) { return pEntity->GetID() == entityID; } );
        if ( entityIdx != InvalidIndex )
        {
            pEntityToDestroy = m_entitiesToAdd[entityIdx];
            m_entitiesToAdd.erase_unsorted( m_entitiesToAdd.begin() + entityIdx );
            KRG::Delete( pEntityToDestroy );
        }
        else
        {
            // Queue removal
            if ( m_isMapInstantiated )
            {
                pEntityToDestroy = FindEntity( entityID );
                KRG_ASSERT( pEntityToDestroy != nullptr );
                m_entitiesToRemove.emplace_back( RemovalRequest( pEntityToDestroy, true ) );
            }
            else// Unknown entity
            {
                KRG_UNREACHABLE_CODE();
            }
        }
    }

    void EntityMap::DestroyAllEntities()
    {
        for ( auto& pEntity : m_entities )
        {
            KRG::Delete( pEntity );
        }

        m_entities.clear();
        m_entityIDLookupMap.clear();

        #if KRG_DEVELOPMENT_TOOLS
        m_entityNameLookupMap.clear();
        #endif
    }

    //-------------------------------------------------------------------------

    void EntityMap::Load( EntityLoadingContext const& loadingContext )
    {
        KRG_ASSERT( Threading::IsMainThread() && loadingContext.IsValid() );
        KRG_ASSERT( m_status == Status::Unloaded );

        Threading::RecursiveScopeLock lock( m_mutex );

        if ( m_isTransientMap )
        {
            m_status = Status::Loaded;
            m_isMapInstantiated = true;
        }
        else // Request loading of map resource
        {
            loadingContext.m_pResourceSystem->LoadResource( m_pMapDesc );
            m_status = Status::MapDescriptorLoading;
        }
    }

    void EntityMap::Unload( EntityLoadingContext const& loadingContext )
    {
        KRG_ASSERT( m_status != Status::Unloaded );
        Threading::RecursiveScopeLock lock( m_mutex );
        m_isUnloadRequested = true;
    }

    void EntityMap::Activate( EntityModel::ActivationContext& activationContext )
    {
        KRG_PROFILE_SCOPE_SCENE( "Map Activation" );
        KRG_ASSERT( m_status == Status::Loaded );

        //-------------------------------------------------------------------------

        struct EntityActivationTask : public ITaskSet
        {
            EntityActivationTask( EntityModel::ActivationContext& activationContext, TVector<Entity*>& entities )
                : m_activationContext( activationContext )
                , m_entities( entities )
            {
                m_SetSize = (uint32) m_entities.size();
            }

            virtual void ExecuteRange( TaskSetPartition range, uint32 threadnum ) override final
            {
                KRG_PROFILE_SCOPE_SCENE( "Activate Entities Task" );

                for ( uint64 i = range.start; i < range.end; ++i )
                {
                    auto pEntity = m_entities[i];
                    if ( pEntity->IsLoaded() )
                    {
                        // Only activate non-spatial and root spatial entities
                        if ( !pEntity->IsSpatialEntity() || !pEntity->HasSpatialParent() )
                        {
                            pEntity->Activate( m_activationContext );
                        }
                    }
                }
            }

        private:

            EntityModel::ActivationContext&         m_activationContext;
            TVector<Entity*>&                       m_entities;
        };

        //-------------------------------------------------------------------------

        Threading::RecursiveScopeLock lock( m_mutex );

        EntityActivationTask activationTask( activationContext, m_entities );
        activationContext.m_pTaskSystem->ScheduleTask( &activationTask );
        activationContext.m_pTaskSystem->WaitForTask( &activationTask );

        m_status = Status::Activated;
    }

    void EntityMap::Deactivate( EntityModel::ActivationContext& activationContext )
    {
        KRG_PROFILE_SCOPE_SCENE( "Map Deactivation" );
        KRG_ASSERT( m_status == Status::Activated );

        //-------------------------------------------------------------------------

        struct EntityDeactivationTask : public ITaskSet
        {
            EntityDeactivationTask( EntityModel::ActivationContext& activationContext, TVector<Entity*>& entities )
                : m_activationContext( activationContext )
                , m_entities( entities )
            {
                m_SetSize = (uint32) m_entities.size();
            }

            virtual void ExecuteRange( TaskSetPartition range, uint32 threadnum ) override final
            {
                KRG_PROFILE_SCOPE_SCENE( "Deactivate Entities Task" );

                for ( uint64 i = range.start; i < range.end; ++i )
                {
                    auto pEntity = m_entities[i];
                    if ( pEntity->IsActivated() )
                    {
                        if ( !pEntity->IsSpatialEntity() || !pEntity->HasSpatialParent() )
                        {
                            pEntity->Deactivate( m_activationContext );
                        }
                    }
                }
            }

        private:

            EntityModel::ActivationContext&         m_activationContext;
            TVector<Entity*>&                       m_entities;
        };

        //-------------------------------------------------------------------------

        Threading::RecursiveScopeLock lock( m_mutex );

        EntityDeactivationTask deactivationTask( activationContext, m_entities );
        activationContext.m_pTaskSystem->ScheduleTask( &deactivationTask );
        activationContext.m_pTaskSystem->WaitForTask( &deactivationTask );

        m_status = Status::Loaded;
    }

    //-------------------------------------------------------------------------

    void EntityMap::OnEntityStateUpdated( Entity* pEntity )
    {
        if ( pEntity->GetMapID() == m_ID )
        {
            KRG_ASSERT( FindEntity( pEntity->GetID() ) );
            Threading::RecursiveScopeLock lock( m_mutex );
            KRG_ASSERT( !VectorContains( m_entitiesCurrentlyLoading, pEntity ) );
            m_entitiesCurrentlyLoading.emplace_back( pEntity );
        }
    }

    void EntityMap::AddEntityCollection( TaskSystem* pTaskSystem, TypeSystem::TypeRegistry const& typeRegistry, EntityCollectionDescriptor const& entityCollectionDesc, Transform const& offsetTransform )
    {
        TVector<Entity*> const createdEntities = entityCollectionDesc.InstantiateCollection( pTaskSystem, typeRegistry );
        AddEntities( createdEntities, offsetTransform );
    }

    //-------------------------------------------------------------------------

    bool EntityMap::ProcessMapUnloadRequest( EntityLoadingContext const& loadingContext, EntityModel::ActivationContext& activationContext )
    {
        KRG_ASSERT( m_isUnloadRequested );

        //-------------------------------------------------------------------------

        // Ensure that we also deactivate all entities properly
        if ( IsActivated() )
        {
            Deactivate( activationContext );
        }
        else
        {
            if ( m_status != Status::LoadFailed )
            {
                if ( m_isMapInstantiated )
                {
                    // Unload entities
                    for ( auto pEntity : m_entities )
                    {
                        KRG_ASSERT( !pEntity->IsActivated() );
                        pEntity->UnloadComponents( loadingContext );
                    }

                    // Delete instantiated entities
                    DestroyAllEntities();
                    m_isMapInstantiated = false;
                }

                // Since entity ownership is transferred via the add call, we need to delete all pending add entity requests
                for ( auto pEntity : m_entitiesToAdd )
                {
                    KRG_ASSERT( !pEntity->HasRequestedComponentLoad() );
                    KRG::Delete( pEntity );
                }
                m_entitiesToAdd.clear();

                // Clear all internal entity lists
                m_entitiesCurrentlyLoading.clear();
                m_entitiesToRemove.clear();
            }

            // Unload the map resource
            if ( !m_isTransientMap && !m_pMapDesc.IsUnloaded() )
            {
                loadingContext.m_pResourceSystem->UnloadResource( m_pMapDesc );
            }

            m_status = Status::Unloaded;
            m_isUnloadRequested = false;
            return true;
        }

        return false;
    }

    bool EntityMap::ProcessMapLoading( EntityLoadingContext const& loadingContext, EntityModel::ActivationContext& activationContext )
    {
        KRG_ASSERT( m_status == Status::MapDescriptorLoading );
        KRG_ASSERT( !m_isTransientMap );

        if ( m_pMapDesc.IsLoading() )
        {
            return false;
        }

        //-------------------------------------------------------------------------

        if ( m_pMapDesc.HasLoadingFailed() )
        {
            m_status = Status::LoadFailed;
        }
        else if ( m_pMapDesc.IsLoaded() )
        {
            if ( m_pMapDesc->IsValid() )
            {
                // Create entities
                TVector<Entity*> const createdEntities = m_pMapDesc.GetPtr()->InstantiateCollection( loadingContext.m_pTaskSystem, *loadingContext.m_pTypeRegistry );

                // Add and load all entities
                m_entities.reserve( m_entities.size() + createdEntities.size() );
                m_entityIDLookupMap.reserve( m_entityIDLookupMap.size() + createdEntities.size() );
                m_entitiesCurrentlyLoading.reserve( m_entitiesCurrentlyLoading.size() + m_pMapDesc.GetPtr()->GetNumEntityDescriptors() );

                for ( auto pEntity : createdEntities )
                {
                    pEntity->m_mapID = m_ID;
                    m_entities.emplace_back( pEntity );
                    AddEntityToLookupMaps( pEntity );

                    // Request load
                    pEntity->LoadComponents( loadingContext );
                    KRG_ASSERT( !VectorContains( m_entitiesCurrentlyLoading, pEntity ) );
                    m_entitiesCurrentlyLoading.emplace_back( pEntity );
                }

                m_isMapInstantiated = true;
                m_status = Status::MapEntitiesLoading;
            }
            else // Invalid map data is treated as a failed load
            {
                m_status = Status::LoadFailed;
            }
        }

        // Release map resource ptr once loading has completed
        loadingContext.m_pResourceSystem->UnloadResource( m_pMapDesc );
        return true;
    }

    void EntityMap::ProcessEntityAdditionAndRemoval( EntityLoadingContext const& loadingContext, EntityModel::ActivationContext& activationContext )
    {
        // Edited Entities
        //-------------------------------------------------------------------------

        #if KRG_DEVELOPMENT_TOOLS
        for ( auto pEntity : m_editedEntities )
        {
            pEntity->EndComponentEditing( loadingContext );
            KRG_ASSERT( !VectorContains( m_entitiesCurrentlyLoading, pEntity ) );
            m_entitiesCurrentlyLoading.emplace_back( pEntity );
        }
        m_editedEntities.clear();
        #endif

        // Addition
        //-------------------------------------------------------------------------

        // Wait until we have a collection to add the entities too since the map might still be loading
        if ( m_isMapInstantiated )
        {
            // Add entities to the collection and request load
            for ( auto pEntityToAdd : m_entitiesToAdd )
            {
                // Ensure that the entity to add, is not already part of a collection and that it's deactivated
                KRG_ASSERT( pEntityToAdd != nullptr && pEntityToAdd->m_mapID == m_ID && !pEntityToAdd->IsActivated() );
                KRG_ASSERT( ContainsEntity( pEntityToAdd->m_ID ) );

                m_entities.push_back( pEntityToAdd );

                pEntityToAdd->LoadComponents( loadingContext );
                KRG_ASSERT( !VectorContains( m_entitiesCurrentlyLoading, pEntityToAdd ) );
                m_entitiesCurrentlyLoading.emplace_back( pEntityToAdd );
            }

            m_entitiesToAdd.clear();
        }

        // Removal
        //-------------------------------------------------------------------------

        // Unload and deactivate entities and remove them from the collection
        for ( int32 i = (int32) m_entitiesToRemove.size() - 1; i >= 0; i-- )
        {
            auto& removalRequest = m_entitiesToRemove[i];
            auto pEntityToRemove = removalRequest.m_pEntity;

            // Deactivate if activated
            if ( pEntityToRemove->IsActivated() )
            {
                pEntityToRemove->Deactivate( activationContext );
                continue;
            }
            else // Remove from loading list as we might still be loaded this entity
            {
                m_entitiesCurrentlyLoading.erase_first_unsorted( pEntityToRemove );

                // Unload components and remove from collection
                pEntityToRemove->UnloadComponents( loadingContext );
                pEntityToRemove->m_mapID.Clear();
                RemoveEntityFromLookupMaps( pEntityToRemove );
                m_entities.erase_first( pEntityToRemove );

                // Destroy the entity if this is a destruction request
                if ( removalRequest.m_shouldDestroy )
                {
                    KRG::Delete( pEntityToRemove );
                }

                // Remove the request from the list
                m_entitiesToRemove.erase_unsorted( m_entitiesToRemove.begin() + i );
            }
        }
    }

    bool EntityMap::ProcessEntityLoadingAndActivation( EntityLoadingContext const& loadingContext, EntityModel::ActivationContext& activationContext )
    {
        struct EntityLoadingTask : public ITaskSet
        {
            EntityLoadingTask( EntityLoadingContext const& loadingContext, EntityModel::ActivationContext& activationContext, TVector<Entity*>& entitiesToLoad, bool isActivated )
                : m_loadingContext( loadingContext )
                , m_activationContext( activationContext )
                , m_entitiesToLoad( entitiesToLoad )
                , m_isActivated( isActivated )
            {
                m_SetSize = (uint32) m_entitiesToLoad.size();
            }

            virtual void ExecuteRange( TaskSetPartition range, uint32 threadnum ) override final
            {
                KRG_PROFILE_SCOPE_SCENE( "Load and Activate Entities Task" );

                for ( uint32 i = range.start; i < range.end; ++i )
                {
                    auto pEntity = m_entitiesToLoad[i];

                    if ( pEntity->UpdateEntityState( m_loadingContext, m_activationContext ) )
                    {
                        for ( auto pComponent : pEntity->GetComponents() )
                        {
                            KRG_ASSERT( pComponent->IsInitialized() || pComponent->HasLoadingFailed() );
                        }

                        // If the map is activated, immediately activate any entities that finish loading
                        if ( m_isActivated && pEntity->IsLoaded() )
                        {
                            // Prevent us from activating entities whose parents are not yet activated, this ensures that our attachment chain have a consistent activation state
                            if ( !pEntity->HasSpatialParent() || pEntity->GetSpatialParent()->IsActivated() )
                            {
                                pEntity->Activate( m_activationContext );
                            }
                        }
                    }
                    else // Entity is still loading
                    {
                        bool result = m_stillLoadingEntities.enqueue( pEntity );
                        KRG_ASSERT( result );
                    }
                }
            }

        public:

            Threading::LockFreeQueue<Entity*>       m_stillLoadingEntities;

        private:

            EntityLoadingContext const&             m_loadingContext;
            EntityModel::ActivationContext&         m_activationContext;
            TVector<Entity*>&                       m_entitiesToLoad;
            bool                                    m_isActivated = false;
        };

        //-------------------------------------------------------------------------

        {
            KRG_PROFILE_SCOPE_SCENE( "Load and Activate Entities" );

            //-------------------------------------------------------------------------

            EntityLoadingTask loadingTask( loadingContext, activationContext, m_entitiesCurrentlyLoading, IsActivated() );
            loadingContext.m_pTaskSystem->ScheduleTask( &loadingTask );
            loadingContext.m_pTaskSystem->WaitForTask( &loadingTask );

            //-------------------------------------------------------------------------

            // Track the number of entities that still need loading
            size_t const numEntitiesStillLoading = loadingTask.m_stillLoadingEntities.size_approx();
            m_entitiesCurrentlyLoading.resize( numEntitiesStillLoading );
            size_t numDequeued = loadingTask.m_stillLoadingEntities.try_dequeue_bulk( m_entitiesCurrentlyLoading.data(), numEntitiesStillLoading );
            KRG_ASSERT( numEntitiesStillLoading == numDequeued );
        }

        //-------------------------------------------------------------------------

        // Ensure that we set the status to loaded, if we were in the entity loading stage and all loads completed
        if ( m_status == Status::MapEntitiesLoading && m_entitiesCurrentlyLoading.empty() )
        {
            KRG_ASSERT( !m_isTransientMap );
            m_status = Status::Loaded;
        }

        //-------------------------------------------------------------------------

        return m_entitiesCurrentlyLoading.empty();
    }

    //-------------------------------------------------------------------------

    bool EntityMap::UpdateState( EntityLoadingContext const& loadingContext, EntityModel::ActivationContext& activationContext )
    {
        KRG_PROFILE_SCOPE_SCENE( "Map Loading" );
        KRG_ASSERT( Threading::IsMainThread() && loadingContext.IsValid() );

        #if KRG_DEVELOPMENT_TOOLS
        KRG_ASSERT( m_entitiesToHotReload.empty() );
        #endif

        //-------------------------------------------------------------------------

        Threading::RecursiveScopeLock lock( m_mutex );

        //-------------------------------------------------------------------------

        // Process the request and return immediately if it isnt completed
        if ( m_isUnloadRequested )
        {
            if ( !ProcessMapUnloadRequest( loadingContext, activationContext ) )
            {
                return false;
            }
        }

        //-------------------------------------------------------------------------

        // Wait for the map descriptor to finish loading
        if ( m_status == Status::MapDescriptorLoading )
        {
            if ( !ProcessMapLoading( loadingContext, activationContext ) )
            {
                return false;
            }
        }

        //-------------------------------------------------------------------------

        ProcessEntityAdditionAndRemoval( loadingContext, activationContext );
        return ProcessEntityLoadingAndActivation( loadingContext, activationContext );
    }

    //-------------------------------------------------------------------------

    #if KRG_DEVELOPMENT_TOOLS
    void EntityMap::ComponentEditingDeactivate( EntityModel::ActivationContext& activationContext, EntityID const& entityID, ComponentID const& componentID )
    {
        KRG_ASSERT( Threading::IsMainThread() );

        auto pEntity = FindEntity( entityID );
        KRG_ASSERT( pEntity != nullptr );
        pEntity->ComponentEditingDeactivate( activationContext, componentID );

        {
            Threading::RecursiveScopeLock lock( m_mutex );
            if ( std::find( m_editedEntities.begin(), m_editedEntities.end(), pEntity ) == m_editedEntities.end() )
            {
                m_editedEntities.emplace_back( pEntity );
            }
        }
    }

    void EntityMap::ComponentEditingUnload( EntityLoadingContext const& loadingContext, EntityID const& entityID, ComponentID const& componentID )
    {
        KRG_ASSERT( Threading::IsMainThread() );

        auto pEntity = FindEntity( entityID );
        KRG_ASSERT( pEntity != nullptr );
        KRG_ASSERT( VectorContains( m_editedEntities, pEntity ) );
        pEntity->ComponentEditingUnload( loadingContext, componentID );
    }

    //-------------------------------------------------------------------------

    void EntityMap::HotReloadDeactivateEntities( EntityModel::ActivationContext& activationContext, TVector<Resource::ResourceRequesterID> const& usersToReload )
    {
        KRG_ASSERT( Threading::IsMainThread() );
        KRG_ASSERT( !usersToReload.empty() );
        KRG_ASSERT( m_entitiesToHotReload.empty() );

        // Generate list of entities to be reloaded
        for ( auto const& requesterID : usersToReload )
        {
            // See if the entity that needs a reload is in this map
            Entity* pFoundEntity = FindEntity( EntityID( requesterID.GetID() ) );
            if ( pFoundEntity != nullptr )
            {
                m_entitiesToHotReload.emplace_back( pFoundEntity );
            }
        }

        // Request deactivation for any entities that are activated
        for ( auto pEntityToHotReload : m_entitiesToHotReload )
        {
            if ( pEntityToHotReload->IsActivated() )
            {
                pEntityToHotReload->Deactivate( activationContext );
            }
        }
    }

    void EntityMap::HotReloadUnloadEntities( EntityLoadingContext const& loadingContext )
    {
        KRG_ASSERT( Threading::IsMainThread() );

        Threading::RecursiveScopeLock lock( m_mutex );

        for ( auto pEntityToHotReload : m_entitiesToHotReload )
        {
            KRG_ASSERT( !pEntityToHotReload->IsActivated() );

            // We might still be loading this entity so remove it from the loading requests
            m_entitiesCurrentlyLoading.erase_first_unsorted( pEntityToHotReload );

            // Request unload of the components (client system needs to ensure that all resource requests are processed)
            pEntityToHotReload->UnloadComponents( loadingContext );
        }
    }

    void EntityMap::HotReloadLoadEntities( EntityLoadingContext const& loadingContext )
    {
        KRG_ASSERT( Threading::IsMainThread() );

        Threading::RecursiveScopeLock lock( m_mutex );

        for ( auto pEntityToHotReload : m_entitiesToHotReload )
        {
            KRG_ASSERT( pEntityToHotReload->IsUnloaded() );
            pEntityToHotReload->LoadComponents( loadingContext );
            m_entitiesCurrentlyLoading.emplace_back( pEntityToHotReload );
        }

        m_entitiesToHotReload.clear();
    }

    //-------------------------------------------------------------------------

    static InlineString GenerateUniqueName( InlineString const& baseName, int32 counterValue )
    {
        InlineString finalName;

        if ( baseName.length() > 3 )
        {
            // Check if the last three characters are a numeric set, if so then increment the value and replace them
            if ( isdigit( baseName[baseName.length() - 1] ) && isdigit( baseName[baseName.length() - 2] ) && isdigit( baseName[baseName.length() - 3] ) )
            {
                finalName.sprintf( "%s%03u", baseName.substr( 0, baseName.length() - 3 ).c_str(), counterValue );
                return finalName;
            }
        }

        finalName.sprintf( "%s %03u", baseName.c_str(), counterValue );
        return finalName;
    }

    StringID EntityMap::GenerateUniqueEntityName( StringID desiredNameID ) const
    {
        KRG_ASSERT( desiredNameID.IsValid() );

        InlineString desiredName = desiredNameID.c_str();
        InlineString finalName = desiredName;
        StringID finalNameID( finalName.c_str() );

        uint32 counter = 0;
        bool isUniqueName = false;
        while ( !isUniqueName )
        {
            isUniqueName = m_entityNameLookupMap.find( finalNameID ) == m_entityNameLookupMap.end();

            // If we found a name clash, generate a new name and try again
            if ( !isUniqueName )
            {
                finalName = GenerateUniqueName( desiredName, counter );
                finalNameID = StringID( finalName.c_str() );
                counter++;
            }
        }

        //-------------------------------------------------------------------------

        return finalNameID;
    }
    #endif
}