#pragma once

#include "EntityComponent.h"
#include "System/Core/Math/BoundingVolumes.h"
#include "System/Core/Math/Transform.h"
#include "System/TypeSystem/TypeRegistrationMacros.h"

//-------------------------------------------------------------------------

namespace KRG
{
    namespace EntityModel { class EntityCollection; }

    //-------------------------------------------------------------------------

    class KRG_SYSTEM_ENTITY_API SpatialEntityComponent : public EntityComponent
    {
        KRG_REGISTER_TYPE;

        friend class Entity;
        friend class EntityDebugViewController;
        friend class EntityModel::EntityCollection;

        struct AttachmentSocketTransformResult
        {
            AttachmentSocketTransformResult( Matrix transform ) : m_transform( transform ) {}

            Matrix      m_transform;
            bool        m_wasFound = false;
        };

    public:

        virtual TypeSystem::TypeInfo const* GetTypeInfo() const override { return SpatialEntityComponent::s_pTypeInfo; }

        // Spatial data
        //-------------------------------------------------------------------------

        inline bool IsRootComponent() const { return m_pSpatialParent == nullptr; }
        inline bool HasChildren() const { return !m_spatialChildren.empty(); }

        inline Transform const& GetLocalTransform() const { return m_transform; }
        inline OBB const& GetLocalBounds() const { return m_bounds; }

        inline Transform const& GetWorldTransform() const { return m_worldTransform; }
        inline OBB const& GetWorldBounds() const { return m_worldBounds; }

        // Call to update the local transform - this will also update the world transform for this component and all children
        inline void SetLocalTransform( Transform const& newTransform )
        {
            m_transform = newTransform;
            CalculateWorldTransform();
        }

        // Call to update the world transform - this will also updated the local transform for this component and all children's world transforms
        inline void SetWorldTransform( Transform const& newTransform )
        {
            SetWorldTransformDirectly( newTransform );
        }

        // The socket that this component is attached to
        inline StringID GetAttachmentSocketID() const { return m_parentAttachmentSocketID; }
        inline void SetAttachmentSocketID( StringID socketID ) { m_parentAttachmentSocketID = socketID; }

        // Returns the world transform for the specified attachment socket if it exists, if it doesnt this function returns the world transform
        // The search children parameter controls, whether to only search this component or to also search it's children
        Transform GetAttachmentSocketTransform( StringID socketID ) const;

    protected:

        using EntityComponent::EntityComponent;

        // Call to update the local bounds - this will ONLY update the world bounds for this component
        inline void SetLocalBounds( OBB const& newBounds )
        {
            m_bounds = newBounds;
            m_worldBounds = m_bounds.GetTransformed( m_worldTransform );
        }

        // Try to find and return the world space transform for the specified socket
        bool TryGetAttachmentSocketTransform( StringID socketID, Transform& outSocketWorldTransform ) const;

        // This should be implemented on each derived spatial component to find the transform of the socket if it exists
        // This function must always return a valid world transform in the outSocketTransform usually that of the component
        virtual bool TryFindAttachmentSocketTransform( StringID socketID, Transform& outSocketWorldTransform ) const;

        // This function should be implemented on any component that supports sockets, it will check if the specified socket exists on the component
        virtual bool HasSocket( StringID socketID ) const { return false; }

        // This function should be called whenever a socket is created/destroyed or its position is updated
        void NotifySocketsUpdated();

        // Called whenever the world transform is updated, try to avoid doing anything expensive in this function
        virtual void OnWorldTransformUpdated() {}

        // This function allows you to directly set the world transform for a component and skip the callback.
        // This must be used with care and so not be exposed externally.
        inline void SetWorldTransformDirectly( Transform NewWorldTransform, bool triggerCallback = true )
        {
            // Only update the transform if we have a parent, if we dont have a parent it means we are the root transform
            if ( m_pSpatialParent != nullptr )
            {
                auto parentWorldTransform = m_pSpatialParent->GetAttachmentSocketTransform( m_parentAttachmentSocketID );
                m_worldTransform = NewWorldTransform;
                m_transform = m_worldTransform * parentWorldTransform.GetInverse();
            }
            else
            {
                m_worldTransform = NewWorldTransform;
                m_transform = NewWorldTransform;
            }

            // Calculate world bounds
            m_worldBounds = m_bounds.GetTransformed( m_worldTransform );

            // Propagate the world transforms on the children
            for ( auto pChild : m_spatialChildren )
            {
                pChild->CalculateWorldTransform( triggerCallback );
            }

            if ( triggerCallback )
            {
                OnWorldTransformUpdated();
            }
        }
    
    private:

        // Called whenever the local transform is modified
        inline void CalculateWorldTransform( bool triggerCallback = true )
        {
            // Only update the transform if we have a parent, if we dont have a parent it means we are the root transform
            if ( m_pSpatialParent != nullptr )
            {
                auto parentWorldTransform = m_pSpatialParent->GetAttachmentSocketTransform( m_parentAttachmentSocketID );
                m_worldTransform = m_transform * parentWorldTransform;
            }
            else
            {
                m_worldTransform = m_transform;
            }

            // Calculate world bounds
            m_worldBounds = m_bounds.GetTransformed( m_worldTransform );

            // Propagate the world transforms on the children
            for ( auto pChild : m_spatialChildren )
            {
                pChild->CalculateWorldTransform( triggerCallback );
            }

            if ( triggerCallback )
            {
                OnWorldTransformUpdated();
            }
        }

    private:

        EXPOSE Transform                                                    m_transform;                            // Local space transform
        OBB                                                                 m_bounds;                               // Local space bounding box
        Transform                                                           m_worldTransform;                       // World space transform (left uninitialized to catch initialization errors)
        OBB                                                                 m_worldBounds;                          // World space bounding box

        //-------------------------------------------------------------------------

        SpatialEntityComponent*                                             m_pSpatialParent = nullptr;             // The component we are attached to
        StringID                                                            m_parentAttachmentSocketID;             // The socket we are attached to (can be invalid)
        TInlineVector<SpatialEntityComponent*, 2>                           m_spatialChildren;                      // All components that are attached to us
    };
}