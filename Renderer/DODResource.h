#pragma once
#include "DOD.h"
#include <unordered_map>
#include <string>

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

        template<class DataType, uint32_t count>
        struct ResourceManagerBase: ManagerBase<DataType, count>
        {
            static void initResourceManager()
            {
                ManagerBase<DataType, count>::initManager();
            }

			static Ref createResource(const std::string& name)
			{
				Ref ref = ManagerBase<DataType, count>::allocate();
				data.name[ref._id] = name;
				nameResourceMap[name] = ref;
				return ref;
			}

			static std::string GetNameByRef(const DOD::Ref& ref)
			{
				return data.name[ref._id];
			}

			static void destroyResource(const Ref& ref)
			{
				nameResourceMap.erase(GetNameByRef(ref));
				ManagerBase<DataType, count>::release(ref);
			}

            static std::unordered_map<std::string, Ref> nameResourceMap;
            static DataType data;
            static std::string resourceName;
        };

		template<class DataType, uint32_t count>
		std::unordered_map<std::string, Ref>
		ResourceManagerBase<DataType, count>::nameResourceMap;

		template<class DataType, uint32_t count>
		DataType
		ResourceManagerBase<DataType, count>::data;

		template<class DataType, uint32_t count>
		std::string
		ResourceManagerBase<DataType, count>::resourceName;
    }
}