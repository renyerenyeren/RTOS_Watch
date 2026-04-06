//
// Created by redmiX on 2026/4/6.
//

//******************************** Includes *********************************//
#include "dlist.h"
#include <stddef.h>
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Macros ***********************************//
//******************************** Macros ***********************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明   *********************************//
static dnode_t* alloc_node(dlist_t* list);
static void free_node(dlist_t* list, dnode_t* node);
//******************************** 函数声明   *********************************//
//---------------------------------------------------------------------------//
//******************************** Variables ********************************//
//******************************** Variables ********************************//
//---------------------------------------------------------------------------//
//******************************** Functions ********************************//
/**
 * @brief  从节点池分配一个空闲节点
 * @param  list: 链表实例
 * @return 节点指针/NULL
 */
static dnode_t* alloc_node(dlist_t* list)
{
    if (NULL == list || NULL == list->freeList)
    {
        return NULL;
    }
    dnode_t* node = list->freeList;
    list->freeList = list->freeList->next;

    // 初始化节点
    node->data = NULL;
    node->next = NULL;
    node->prev = NULL;

    return node;
}

/**
 * @brief  归还节点到空闲池
 * @param  list: 链表实例
 * @param  node: 待释放节点
 */
static void free_node(dlist_t* list, dnode_t* node)
{
    if (NULL == list || NULL == node)
    {
        return;
    }

    node->next = list->freeList;
    list->freeList = node;
}

/**
 * @brief 初始化双向链表
 * @param list 待初始化的链表指针
 * @return DLIST_OK 成功
 *         DLIST_ERR_NULL 参数为空
 */
dlist_status_t dListInit(dlist_t* list)
{
    // 1. 检查参数有效性
    if (NULL == list)
    {
        return DLIST_ERR_NULL;
    }

    // 2. 计算内存池中最大节点数
    uint32_t maxnodes = sizeof(list->nodePool) / sizeof(list->nodePool[0]);

    // 3. 初始化内存池，将所有节点链接成空闲链表
    for (uint32_t i = 0; i < maxnodes-1; i++)
    {
        list->nodePool[i].next = &list->nodePool[i+1];
    }
    list->nodePool[maxnodes-1].next = NULL;
    list->freeList = &list->nodePool[0];

    // 4. 初始化链表基本属性
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    list->maxNodes = maxnodes;

    return DLIST_OK;
}

/**
 * @brief 清空链表，释放所有节点回内存池
 * @param list 待清空的链表指针
 */
void dListClear(dlist_t* list)
{
    // 1. 检查参数有效性
    if (NULL == list)
    {
        return;
    }

    // 2. 遍历链表，释放所有节点
    dnode_t* p = list->head;
    while (NULL != p)
    {
        dnode_t* next = p->next;
        free_node(list, p);
        p = next;
    }

    // 3. 重置链表状态
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}

/**
 * @brief 在链表头部插入节点
 * @param list 链表指针
 * @param data 待插入的数据指针
 * @return DLIST_OK 成功
 *         DLIST_ERR_NULL 参数为空
 *         DLIST_ERR_FULL 内存池已满
 */
dlist_status_t dListInsertHead(dlist_t* list, void* data)
{
    // 1. 检查参数有效性
    // if (NULL == list || NULL == data)
    if (NULL == list)  // 允许加入空指针作为占位符
    {
        return DLIST_ERR_NULL;
    }

    // 2. 从内存池分配新节点
    dnode_t* new_node = alloc_node(list);
    if (NULL == new_node)
    {
        return DLIST_ERR_FULL;
    }

    // 3. 设置新节点数据
    new_node->data = data;
    new_node->next = list->head;
    new_node->prev = NULL;

    // 4. 更新链表指针
    if (NULL != list->head)
    {
        list->head->prev = new_node;
    }
    else
    {
        list->tail = new_node;
    }

    list->head = new_node;
    list->length++;

    return DLIST_OK;
}

/**
 * @brief 在链表尾部插入节点
 * @param list 链表指针
 * @param data 待插入的数据指针
 * @return DLIST_OK 成功
 *         DLIST_ERR_NULL 参数为空
 *         DLIST_ERR_FULL 内存池已满
 */
dlist_status_t dListInsertTail(dlist_t* list, void* data)
{
    // 1. 检查参数有效性
    // if (NULL == list || NULL == data)
    if (NULL == list)  // 允许加入空指针作为占位符
    {
        return DLIST_ERR_NULL;
    }

    // 2. 从内存池分配新节点
    dnode_t* new_node = alloc_node(list);
    if (NULL == new_node)
    {
        return DLIST_ERR_FULL;
    }

    // 3. 设置新节点数据
    new_node->data = data;
    new_node->prev = list->tail;
    new_node->next = NULL;

    // 4. 更新链表指针
    if (NULL != list->tail)
    {
        list->tail->next = new_node;
    }
    else
    {
        list->head = new_node;
    }

    list->tail = new_node;
    list->length++;

    return DLIST_OK;
}

/**
 * @brief 在指定位置插入节点
 * @param list 链表指针
 * @param pos 插入位置(0-based)
 * @param data 待插入的数据指针
 * @return DLIST_OK 成功
 *         DLIST_ERR_NULL 参数为空
 *         DLIST_ERR_POS 位置无效
 *         DLIST_ERR_FULL 内存池已满
 */
dlist_status_t dListInsertAt(dlist_t* list, uint32_t pos, void* data)
{
    // 1. 检查参数有效性
    // if (NULL == list || NULL == data)
    if (NULL == list)  // 允许加入空指针作为占位符
    {
        return DLIST_ERR_NULL;
    }
    if (pos > list->length)
    {
        return DLIST_ERR_POS;
    }

    // 2. 处理边界情况：头部和尾部插入
    if (0 == pos)
    {
        return dListInsertHead(list, data);
    }
    if (list->length == pos)
    {
        return dListInsertTail(list, data);
    }

    // 3. 查找插入位置并分配新节点
    dnode_t* pos_node = dListFindByPosition(list, pos);
    dnode_t* new_node = alloc_node(list);

    if (NULL == new_node)
    {
        return DLIST_ERR_FULL;
    }

    // 4. 插入节点并更新指针
    new_node->data = data;
    new_node->prev = pos_node->prev;
    new_node->next = pos_node;
    pos_node->prev->next = new_node;
    pos_node->prev = new_node;
    list->length++;

    return DLIST_OK;
}

/**
 * @brief 删除指定节点
 * @param list 链表指针
 * @param node 待删除的节点指针
 * @return DLIST_OK 成功
 *         DLIST_ERR_NULL 参数为空
 */
dlist_status_t dListDeleteNode(dlist_t* list, dnode_t* node)
{
    // 1. 检查参数有效性
    if (NULL == list || NULL == node)
    {
        return DLIST_ERR_NULL;
    }

    // 2. 更新前驱节点的next指针
    if (NULL != node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        list->head = node->next;
    }

    // 3. 更新后继节点的prev指针
    if (NULL != node->next)
    {
        node->next->prev = node->prev;
    }
    else
    {
        list->tail = node->prev;
    }

    // 4. 释放节点并更新链表长度
    free_node(list, node);
    list->length--;

    return DLIST_OK;
}

/**
 * @brief 根据值删除节点
 * @param list 链表指针
 * @param target_data 目标数据指针
 * @param cmp 比较函数指针
 * @return DLIST_OK 成功
 *         DLIST_ERR_NULL 参数为空
 *         DLIST_ERR_NOT_FOUND 未找到节点
 */
dlist_status_t dListDeleteByValue(dlist_t* list, void* target_data, dlist_cmp_cb_t cmp)
{
    // 1. 检查参数有效性
    if (NULL == list || NULL == target_data || NULL == cmp)
    {
        return DLIST_ERR_NULL;
    }

    // 2. 查找目标节点
    dnode_t* node = dListFindByValue(list, target_data, cmp);
    if (NULL == node)
    {
        return DLIST_ERR_NOT_FOUND;
    }

    // 3. 删除找到的节点
    return dListDeleteNode(list, node);
}

/**
 * @brief 删除指定位置的节点
 * @param list 链表指针
 * @param pos 位置(0-based)
 * @return DLIST_OK 成功
 *         DLIST_ERR_NULL 参数为空
 *         DLIST_ERR_POS 位置无效
 */
dlist_status_t dListDeleteAt(dlist_t* list, uint32_t pos)
{
    // 1. 检查参数有效性
    if (NULL == list)
    {
        return DLIST_ERR_NULL;
    }
    if (pos >= list->length)
    {
        return DLIST_ERR_POS;
    }

    // 2. 查找目标节点
    dnode_t* node = dListFindByPosition(list, pos);
    if (NULL == node)
    {
        return DLIST_ERR_POS;
    }

    // 3. 删除找到的节点
    return dListDeleteNode(list, node);
}

/**
 * @brief 根据值查找节点
 * @param list 链表指针
 * @param target_data 目标数据指针
 * @param cmp 比较函数指针
 * @return 找到的节点指针，未找到返回NULL
 */
dnode_t* dListFindByValue(dlist_t* list, void* target_data, dlist_cmp_cb_t cmp)
{
    // 1. 检查参数有效性
    if (NULL == list || NULL == target_data || NULL == cmp)
    {
        return NULL;
    }

    // 2. 遍历链表查找匹配节点
    dnode_t* p = list->head;
    while (NULL != p)
    {
        if (cmp(p->data, target_data) == 0)
        {
            return p;
        }
        p = p->next;
    }

    return NULL;
}

/**
 * @brief 根据位置查找节点
 * @param list 链表指针
 * @param pos 位置(0-based)
 * @return 找到的节点指针，未找到返回NULL
 */
dnode_t* dListFindByPosition(dlist_t* list, uint32_t pos)
{
    // 1. 检查参数有效性
    if (NULL == list || pos >= list->length)
    {
        return NULL;
    }

    // 2. 遍历到指定位置
    dnode_t* p = list->head;
    for (uint32_t i = 0; i < pos; i++)
    {
        p = p->next;
    }

    return p;
}

/**
 * @brief 反转链表
 * @param list 链表指针
 */
void dListReverse(dlist_t* list)
{
    // 1. 检查参数有效性
    if (list == NULL || list->length <= 1)
    {
        return;
    }

    // 2. 交换每个节点的前后指针
    dnode_t* p = list->head;
    dnode_t* temp = NULL;

    while (p != NULL)
    {
        temp = p->prev;
        p->prev = p->next;
        p->next = temp;
        p = p->prev;
    }

    // 3. 交换头尾指针
    temp = list->head;
    list->head = list->tail;
    list->tail = temp;
}

/**
 * @brief 正向遍历链表
 * @param list 链表指针
 * @param cb 回调函数指针
 * @param userData 用户数据指针
 */
void dListForEachForward(dlist_t* list, dlist_cb_t cb, void *userData)
{
    // 1. 检查参数有效性
    if (list == NULL || cb == NULL)
    {
        return;
    }

    // 2. 从头到尾遍历链表
    dnode_t* p = list->head;
    while (p != NULL)
    {
        // 调用用户传入的回调函数
        if (cb(p, userData) != 0)
        {
            break; // 返回非0则停止遍历
        }

        p = p->next;
    }
}

/**
 * @brief 反向遍历链表
 * @param list 链表指针
 * @param cb 回调函数指针
 * @param userData 用户数据指针
 */
void dListForEachBackward(dlist_t* list, dlist_cb_t cb, void *userData)
{
    // 1. 检查参数有效性
    if (list == NULL || cb == NULL)
    {
        return;
    }

    // 2. 从尾到头遍历链表
    dnode_t* p = list->tail;
    while (p != NULL)
    {
        // 调用用户传入的回调函数
        if (cb(p, userData) != 0)
        {
            break;
        }

        p = p->prev;
    }
}