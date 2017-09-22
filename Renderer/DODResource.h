#pragma once
#include "DOD.h"
#include <unordered_map>

namespace DOD
{
    namespace Resource
    {
        struct ResourceDatabase
        {
            ResourceDatabase(uint32_t size)
            {
                name.resize(size);
                resourceFlags.resize(size);
            }

            std::vector<std::string> name;
            std::vector < uint8_t> resourceFlags;
        };

        template<class DatatType, uint32_t count>
        struct ResourceManagerBase: ResourceBase<DataType, count>
        {
            static void initResourceManager()
            {
                DOD::ManagerBase<DataTpe, count>::initManager();
            }

			static Ref createResource(const std::string& name)
			{
				Ref ref = DOD::ManagerBase<DataType, count>::allocate();
				data.name(ref._id) = name;
				nameResourceMap[name] = ref;
				return ref;
			}

			static destroyResource(const Ref& ref)
			{
				nameResourceMap.erase(ref);
				Dod::ManagerBase<DataType, count>::release(ref);
			}

            static std::unordered_map<std::string, Ref> nameResourceMap;
            static DataType data;
            static std::string resourceName;
        };
    }

    template<class DataType, uint32_t count>
    ResourceManagerBase<DataTpe, count>::nameResourceMap;

    template<class DataType, uint32_t count>
    ResourceManagerBase<DataTpe, count>::data;

    template<class DataType, uint32_t count>
    ResourceManagerBase<DataTpe, count>::resourceName;
}