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

dlist_status_t dListInit(dlist_t* list)
{
    if (NULL == list)
    {
        return DLIST_ERR_NULL;
    }
    uint32_t maxnodes = sizeof(list->nodePool) / sizeof(list->nodePool[0]);

    for (uint32_t i = 0; i < maxnodes-1; i++)
    {
        list->nodePool[i].next = &list->nodePool[i+1];
    }
    list->nodePool[maxnodes-1].next = NULL;
    list->freeList = &list->nodePool[0];

    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    list->maxNodes = maxnodes;

    return DLIST_OK;
}

void dListClear(dlist_t* list)
{
    if (NULL == list)
    {
        return;
    }

    dnode_t* p = list->head;
    while (NULL != p)
    {
        dnode_t* next = p->next;
        free_node(list, p);
        p = next;
    }

    // 重置链表
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
}

dlist_status_t dListInsertHead(dlist_t* list, void* data)
{
    if (NULL == list || NULL == data)
    {
        return DLIST_ERR_NULL;
    }

    dnode_t* new_node = alloc_node(list);
    if (NULL == new_node)
    {
        return DLIST_ERR_FULL;
    }

    new_node->data = data;
    new_node->next = list->head;
    new_node->prev = NULL;

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

dlist_status_t dListInsertTail(dlist_t* list, void* data)
{
    if (NULL == list || NULL == data)
    {
        return DLIST_ERR_NULL;
    }

    dnode_t* new_node = alloc_node(list);
    if (NULL == new_node)
    {
        return DLIST_ERR_FULL;
    }

    new_node->data = data;
    new_node->prev = list->tail;
    new_node->next = NULL;

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

dlist_status_t dListInsertAt(dlist_t* list, uint32_t pos, void* data)
{
    if (NULL == list || NULL == data)
    {
        return DLIST_ERR_NULL;
    }
    if (pos > list->length)
    {
        return DLIST_ERR_POS;
    }

    if (0 == pos)
    {
        return dListInsertHead(list, data);
    }
    if (list->length == pos)
    {
        return dListInsertTail(list, data);
    }

    dnode_t* pos_node = dListFindByPosition(list, pos);
    dnode_t* new_node = alloc_node(list);

    if (NULL == new_node)
    {
        return DLIST_ERR_FULL;
    }

    // 插入节点
    new_node->data = data;
    new_node->prev = pos_node->prev;
    new_node->next = pos_node;
    pos_node->prev->next = new_node;
    pos_node->prev = new_node;
    list->length++;

    return DLIST_OK;
}

dlist_status_t dListDeleteNode(dlist_t* list, dnode_t* node)
{
    if (NULL == list || NULL == node)
    {
        return DLIST_ERR_NULL;
    }

    if (NULL != node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        list->head = node->next;
    }

    if (NULL != node->next)
    {
        node->next->prev = node->prev;
    }
    else
    {
        list->tail = node->prev;
    }

    free_node(list, node);
    list->length--;

    return DLIST_OK;
}

dlist_status_t dListDeleteByValue(dlist_t* list, void* target_data, dlist_cmp_cb_t cmp)
{
    if (NULL == list || NULL == target_data || NULL == cmp)
    {
        return DLIST_ERR_NULL;
    }

    dnode_t* node = dListFindByValue(list, target_data, cmp);
    if (NULL == node)
    {
        return DLIST_ERR_NOT_FOUND;
    }

    return dListDeleteNode(list, node);
}

dlist_status_t dListDeleteAt(dlist_t* list, uint32_t pos)
{
    if (NULL == list)
    {
        return DLIST_ERR_NULL;
    }
    if (pos >= list->length)
    {
        return DLIST_ERR_POS;
    }

    dnode_t* node = dListFindByPosition(list, pos);
    if (NULL == node)
    {
        return DLIST_ERR_POS;
    }

    return dListDeleteNode(list, node);
}

dnode_t* dListFindByValue(dlist_t* list, void* target_data, dlist_cmp_cb_t cmp)
{
    if (NULL == list || NULL == target_data || NULL == cmp)
    {
        return NULL;
    }

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

dnode_t* dListFindByPosition(dlist_t* list, uint32_t pos)
{
    if (NULL == list || pos >= list->length)
    {
        return NULL;
    }

    dnode_t* p = list->head;
    for (uint32_t i = 0; i < pos; i++)
    {
        p = p->next;
    }

    return p;
}

void dListReverse(dlist_t* list)
{
    if (list == NULL || list->length <= 1)
    {
        return;
    }

    dnode_t* p = list->head;
    dnode_t* temp = NULL;

    // 交换每个节点的前后指针
    while (p != NULL)
    {
        temp = p->prev;
        p->prev = p->next;
        p->next = temp;
        p = p->prev;
    }

    // 交换头尾指针
    temp = list->head;
    list->head = list->tail;
    list->tail = temp;
}

void dListForEachForward(dlist_t* list, dlist_cb_t cb, void *userData)
{
    if (list == NULL || cb == NULL)
    {
        return;
    }

    dnode_t* p = list->head;
    while (p != NULL)
    {
        // 调用用户传入的回调函数！
        if (cb(p, userData) != 0)
        {
            break; // 可选：返回非0则停止遍历
        }

        p = p->next;
    }
}

void dListForEachBackward(dlist_t* list, dlist_cb_t cb, void *userData)
{
    if (list == NULL || cb == NULL)
    {
        return;
    }

    dnode_t* p = list->tail;
    while (p != NULL)
    {
        // 调用用户传入的回调函数！
        if (cb(p, userData) != 0)
        {
            break;
        }

        p = p->prev;
    }
}