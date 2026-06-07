module;

#include <vector>

export module core.memory.slot_array;

import core.stdtypes;
import core.memory.handle;

export namespace draco::core::memory
{
    template<typename T>
    struct Slot
    {
        T value{};
        u32 generation = 0;
        bool alive = false;
    };

    template<typename T, typename Tag>
    class SlotArray
    {
    public:
        using HandleType = Handle<Tag>;

        HandleType create(const T& value)
        {
            u32 idx;

            if (!free_list.empty())
            {
                idx = free_list.back();
                free_list.pop_back();
            }
            else
            {
                idx = static_cast<u32>(slots.size());
                slots.push_back({});
            }

            Slot<T>& slot = slots[idx];

            slot.value = value;
            slot.alive = true;

            return HandleType::make(idx, slot.generation);
        }

        bool valid(HandleType h) const
        {
            u32 i = h.index();

            return i < slots.size()
                && slots[i].alive
                && slots[i].generation == h.generation();
        }

        T* get(HandleType h)
        {
            if (!valid(h))
                return nullptr;

            return &slots[h.index()].value;
        }

        const T* get(HandleType h) const
        {
            if (!valid(h))
                return nullptr;

            return &slots[h.index()].value;
        }

        void destroy(HandleType h)
        {
            if (!valid(h))
                return;

            auto& s = slots[h.index()];

            s.alive = false;
            s.generation++;     // invalidate old handles
            free_list.push_back(h.index());
        }

        const std::vector<Slot<T>>& raw() const
        {
            return slots;
        }

    private:
        std::vector<Slot<T>> slots;
        std::vector<u32> free_list;
    };
}
