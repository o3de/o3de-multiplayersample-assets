
#pragma once

#include <AzCore/Component/Component.h>

#include <MPS-Asset-Test/MPS-Asset-TestBus.h>

namespace MPS_Asset_Test
{
    class MPS_Asset_TestSystemComponent
        : public AZ::Component
        , protected MPS_Asset_TestRequestBus::Handler
    {
    public:
        AZ_COMPONENT(MPS_Asset_TestSystemComponent, "{04CFD3D7-735A-4701-85B3-9E4D95D46AA4}");

        static void Reflect(AZ::ReflectContext* context);

        static void GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided);
        static void GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible);
        static void GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required);
        static void GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent);

        MPS_Asset_TestSystemComponent();
        ~MPS_Asset_TestSystemComponent();

    protected:
        ////////////////////////////////////////////////////////////////////////
        // MPS_Asset_TestRequestBus interface implementation

        ////////////////////////////////////////////////////////////////////////

        ////////////////////////////////////////////////////////////////////////
        // AZ::Component interface implementation
        void Init() override;
        void Activate() override;
        void Deactivate() override;
        ////////////////////////////////////////////////////////////////////////
    };
}
