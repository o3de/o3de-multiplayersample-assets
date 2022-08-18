
#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

#include "MPS-Asset-TestSystemComponent.h"

namespace MPS_Asset_Test
{
    void MPS_Asset_TestSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<MPS_Asset_TestSystemComponent, AZ::Component>()
                ->Version(0)
                ;

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<MPS_Asset_TestSystemComponent>("MPS_Asset_Test", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void MPS_Asset_TestSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("MPS_Asset_TestService"));
    }

    void MPS_Asset_TestSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("MPS_Asset_TestService"));
    }

    void MPS_Asset_TestSystemComponent::GetRequiredServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& required)
    {
    }

    void MPS_Asset_TestSystemComponent::GetDependentServices([[maybe_unused]] AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
    }

    MPS_Asset_TestSystemComponent::MPS_Asset_TestSystemComponent()
    {
        if (MPS_Asset_TestInterface::Get() == nullptr)
        {
            MPS_Asset_TestInterface::Register(this);
        }
    }

    MPS_Asset_TestSystemComponent::~MPS_Asset_TestSystemComponent()
    {
        if (MPS_Asset_TestInterface::Get() == this)
        {
            MPS_Asset_TestInterface::Unregister(this);
        }
    }

    void MPS_Asset_TestSystemComponent::Init()
    {
    }

    void MPS_Asset_TestSystemComponent::Activate()
    {
        MPS_Asset_TestRequestBus::Handler::BusConnect();
    }

    void MPS_Asset_TestSystemComponent::Deactivate()
    {
        MPS_Asset_TestRequestBus::Handler::BusDisconnect();
    }
}
