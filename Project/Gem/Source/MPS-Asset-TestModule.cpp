
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

#include "MPS-Asset-TestSystemComponent.h"

namespace MPS_Asset_Test
{
    class MPS_Asset_TestModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(MPS_Asset_TestModule, "{1EC33541-DE80-45AD-97B3-3A902DBF4636}", AZ::Module);
        AZ_CLASS_ALLOCATOR(MPS_Asset_TestModule, AZ::SystemAllocator, 0);

        MPS_Asset_TestModule()
            : AZ::Module()
        {
            // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
            m_descriptors.insert(m_descriptors.end(), {
                MPS_Asset_TestSystemComponent::CreateDescriptor(),
            });
        }

        /**
         * Add required SystemComponents to the SystemEntity.
         */
        AZ::ComponentTypeList GetRequiredSystemComponents() const override
        {
            return AZ::ComponentTypeList{
                azrtti_typeid<MPS_Asset_TestSystemComponent>(),
            };
        }
    };
}// namespace MPS_Asset_Test

AZ_DECLARE_MODULE_CLASS(Gem_MPS_Asset_Test, MPS_Asset_Test::MPS_Asset_TestModule)
