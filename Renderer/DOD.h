#pragma once
#include <cstdint>
#include <vector>
#include <cassert>

namespace DOD
{
    typedef uint8_t GenerationType;
    typedef uint32_t IdType;


    // Note: The last index is used to mark invalid ids and generation ids - thus -2
    static const uint32_t maxGenerationIdValue = 254u; // == 2^8-2
    static const uint32_t maxIdValue = 16777214u;      // == 2^24-2

    // Global constants
    enum
    {
        kInvalidId = maxIdValue + 1u,
        kInvalidGenerationId = maxGenerationIdValue + 1u,
    };

    struct Ref
    {
        Ref() : _generation(kInvalidGenerationId), _id(kInvalidId) {}
        Ref(IdType p_Id, GenerationType p_GenerationId)
            : _generation(p_GenerationId), _id(p_Id)
        {
        }

        bool isValid() const
        {
            return _id != kInvalidId && _generation != kInvalidGenerationId;
        }

		bool operator < (const Ref& p_Rhs) const
		{
			return _id < p_Rhs._id && _generation < p_Rhs._generation;
		}

		bool operator > (const Ref& p_Rhs) const
		{
			return _id > p_Rhs._id && _generation > p_Rhs._generation;
		}

        bool operator==(const Ref& p_Rhs) const
        {
            return _id == p_Rhs._id && _generation == p_Rhs._generation;
        }

        bool operator!=(const Ref& p_Rhs) const
        {
            return !(*this == p_Rhs);
        }

        uint32_t _generation : 8;
        uint32_t _id :  24;
    };

    template<class DataType, uint32_t IdCount>
    struct ManagerBase
    {
        static std::vector<Ref> activeRefs;
		
		static bool isAlive(Ref p_Ref)
		{
			return generations[p_Ref._id] == p_Ref._generation;
		}

		static uint32_t getActiveResourceCount()
		{
			return (uint32_t)activeRefs.size();
		}
		static Ref getActiveResourceAtIndex(uint32_t p_Idx)
		{
			return activeRefs[p_Idx];
		}

	protected:

        static void initManager()
        {
            freeIds.reserve(IdCount);
            activeRefs.reserve(IdCount);
            generations.resize(IdCount);

            for (uint32_t i = 0u; i < IdCount; ++i)
            {
                freeIds.push_back(IdCount - i - 1u);
            }
        }

        static Ref allocate()
        {
            assert(!freeIds.empty() && "Resource pool exhausted");
            uint32_t id = freeIds.back();
            freeIds.pop_back();

            Ref ref;
            ref._id = id;
            ref._generation = generations[id];

            activeRefs.push_back(ref);

            return ref;
        }

        static void release(Ref p_Ref)
        {
            assert(p_Ref.isValid() && isAlive(p_Ref));

            // Erase an swap
            for (uint32_t i = 0; i < activeRefs.size(); ++i)
            {
                if (activeRefs[i] == p_Ref)
                {
                    activeRefs[i] = activeRefs[activeRefs.size() - 1u];
                    activeRefs.resize(activeRefs.size() - 1u);
                    break;
                }
            }

            freeIds.push_back(p_Ref._id);

            const GenerationType currentGenId = generations[p_Ref._id];
            generations[p_Ref._id] = (currentGenId + 1u) % (maxGenerationIdValue + 1u);
        }

        static std::vector<IdType>         freeIds;
        static std::vector<GenerationType> generations;

    };

	//using std::vector<Ref> = RefArray;
}