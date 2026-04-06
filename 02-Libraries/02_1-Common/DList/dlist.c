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
}

/**
 * @brief  归还节点到空闲池
 * @param  list: 链表实例
 * @param  node: 待释放节点
 */
static void free_node(dlist_t* list, dnode_t* node)
{
    if (NULL == list || NULL == list->freeList)
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