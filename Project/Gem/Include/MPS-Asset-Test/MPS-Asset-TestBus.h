
#pragma once

#include <AzCore/EBus/EBus.h>
#include <AzCore/Interface/Interface.h>

namespace MPS_Asset_Test
{
    class MPS_Asset_TestRequests
    {
    public:
        AZ_RTTI(MPS_Asset_TestRequests, "{C0BA4AD3-AA19-4F34-9147-CD9D1A3E6A71}");
        virtual ~MPS_Asset_TestRequests() = default;
        // Put your public methods here
    };

    class MPS_Asset_TestBusTraits
        : public AZ::EBusTraits
    {
    public:
        //////////////////////////////////////////////////////////////////////////
        // EBusTraits overrides
        static constexpr AZ::EBusHandlerPolicy HandlerPolicy = AZ::EBusHandlerPolicy::Single;
        static constexpr AZ::EBusAddressPolicy AddressPolicy = AZ::EBusAddressPolicy::Single;
        //////////////////////////////////////////////////////////////////////////
    };

    using MPS_Asset_TestRequestBus = AZ::EBus<MPS_Asset_TestRequests, MPS_Asset_TestBusTraits>;
    using MPS_Asset_TestInterface = AZ::Interface<MPS_Asset_TestRequests>;

} // namespace MPS_Asset_Test
