#pragma once

#include <cstddef> // for size_t
#include <cstdint> // for uint32_t

// The numbers below represent the index in the memory block containing the
// virtual tables of all the objects This table can be found by following the
// pattern in the get_virtual_function_address function, which looks up the
// pointer to the first in the table.

// These numbers will change with every release of Spelunky. To look them up,
// you can use the x64dbg plugin: make sure you are in a level containing the
// entity you want to detect, click on Virtual Table -> Detect entities and
// search in the resulting list for the entity name, or use the Gather
// functionality.

// Finding the relative offsets can be done by right-clicking somewhere in the
// function you are working in and choosing Spelunky2 > Lookup in virtual table.
// Then, choose the offset to the most sensible base entity type, e.g. MONS_MOLE
// +2 if you're doing mole stuff.

enum class VIRT_FUNC {};

enum class VTABLE_OFFSET {};

size_t get_virtual_function_address(VTABLE_OFFSET table_entry,
                                    uint32_t function_index);
size_t get_virtual_function_address(void *object, uint32_t function_index);
