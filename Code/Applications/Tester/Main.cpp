#include "System/TypeSystem/TypeRegistry.h"
#include "Applications/Shared/ApplicationGlobalState.h"
#include "Applications/Shared/_AutoGenerated/ToolsTypeRegistration.h"
#include "Engine/Physics/PhysicsLayers.h"
#include "System/Core/FileSystem/FileSystem.h"
#include "System/Core/Serialization/JsonArchive.h"
#include "System/Core/Math/NumericRange.h"
#include "System/Core/Types/Event.h"

//-------------------------------------------------------------------------

using namespace KRG;
using namespace KRG::Physics;

//-------------------------------------------------------------------------

static StringID ID( "S" );

int main( int argc, char *argv[] )
{
    {
        KRG::ApplicationGlobalState State;
        TypeSystem::TypeRegistry typeRegistry;
        AutoGenerated::Tools::RegisterTypes( typeRegistry );

        //-------------------------------------------------------------------------

        AutoGenerated::Tools::UnregisterTypes( typeRegistry );
    }

    return 0;
}