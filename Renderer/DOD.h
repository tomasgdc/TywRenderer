#pragma once
#include <cstdint>
#include <vector>

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

        bool operator==(const Ref& p_Rhs) const
        {
            return _id == p_Rhs._id && _generation == p_Rhs._generation;
        }

        bool operator!=(const Ref& p_Rhs) const
        {
            return !(*this == p_Rhs);
        }

        uint32_t _generation : 8;
        uint32_t _id : 24;
    };

    template<class DataType, uint32_t iDcount>
    struct ManagerBase
    {
        static std::vector(Ref)                activeRefs;

    protected:
        static void initManager()
        {
            _freeIds.reserve(IdCount);
            _activeRefs.reserve(IdCount);
            _generations.resize(IdCount);

            for (uint32_t i = 0u; i < IdCount; ++i)
            {
                _freeIds.push_back(IdCount - i - 1u);
            }
        }

        static Ref allocate()
        {
            assert(!_freeIds.empty() && "Resource pool exhausted");
            uint32_t id = _freeIds.back();
            _freeIds.pop_back();

            Ref ref;
            ref._id = id;
            ref._generation = _generations[id];

            _activeRefs.push_back(ref);

            return ref;
        }

        static void release(Ref p_Ref)
        {
            assert(p_Ref.isValid() && isAlive(p_Ref));

            // Erase an swap
            for (uint32_t i = 0; i < _activeRefs.size(); ++i)
            {
                if (_activeRefs[i] == p_Ref)
                {
                    _activeRefs[i] = _activeRefs[_activeRefs.size() - 1u];
                    _activeRefs.resize(_activeRefs.size() - 1u);
                    break;
                }
            }

            _freeIds.push_back(p_Ref._id);

            const GenerationType currentGenId = _generations[p_Ref._id];
            _generations[p_Ref._id] = (currentGenId + 1u) % (maxGenerationIdValue + 1u);
        }

        static std::vector<IdType>         freeIds;
        static std::vector<GenerationType> generations;

    };
}