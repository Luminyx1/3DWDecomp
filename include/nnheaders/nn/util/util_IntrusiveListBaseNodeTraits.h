#pragma once
#include <nn/util/util_IntrusiveList.h>

namespace nn::util {
/**
 * @brief Adapt an inherited list node to a typed intrusive list.
 * @tparam T Item type stored in the list.
 * @tparam Base Base subobject containing the inherited IntrusiveListNode.
 */
template <class T, class Base = T> class IntrusiveListBaseNodeTraits {
  public:
    /**
     * @brief Obtain an item's inherited list node.
     * @param rItem Item whose node is requested.
     * @return Mutable list node belonging to the item.
     */
    static IntrusiveListNode& GetNode(T& rItem) { return static_cast<Base&>(rItem); }
    /**
     * @brief Obtain an item's inherited list node without modifying it.
     * @param rItem Item whose node is requested.
     * @return Read-only list node belonging to the item.
     */
    static const IntrusiveListNode& GetNode(const T& rItem) { return static_cast<const Base&>(rItem); }
    /**
     * @brief Recover an item from its inherited node.
     * @param rNode Node belonging to a T object; must not be the list sentinel.
     * @return Item containing the node.
     */
    static T& GetItem(IntrusiveListNode& rNode) { return static_cast<T&>(static_cast<Base&>(rNode)); }
    /**
     * @brief Recover an item from its inherited node without modifying it.
     * @param rNode Node belonging to a T object; must not be the list sentinel.
     * @return Read-only item containing the node.
     */
    static const T& GetItem(const IntrusiveListNode& rNode) {
        return static_cast<const T&>(static_cast<const Base&>(rNode));
    }
};
} // namespace nn::util
