//
// Created by redmiX on 2026/4/6.
//

#ifndef RTOS_PROJECT_DLIST_H
#define RTOS_PROJECT_DLIST_H

//******************************** Includes *********************************//
#include <stdint.h>
//******************************** Includes *********************************//
//---------------------------------------------------------------------------//
//******************************** Defines **********************************//
#define DLIST_MAX_NODES    20      // 每个链表的最大节点数（可配置）
//******************************** Defines **********************************//
//---------------------------------------------------------------------------//
//******************************** Typedefs *********************************//
typedef enum
{
    DLIST_OK         = 0,// 成功
    DLIST_ERR_NULL,      // 空指针
    DLIST_ERR_FULL,      // 节点池已满
    DLIST_ERR_POS,       // 非法位置
    DLIST_ERR_NOT_FOUND  // 未找到节点
} dlist_status_t;
//******************************** Typedefs *********************************//
//---------------------------------------------------------------------------//
//**************************** Interface Structs ****************************//
/**
 * @brief 双向链表节点结构
 */
typedef struct dnode
{
    void *data;              // 节点数据指针
    struct dnode *next;      // 后继节点指针
    struct dnode *prev;      // 前驱节点指针
} dnode_t;

/**
 * @brief 链表遍历回调函数类型
 * @param node 当前节点
 * @param userData 用户自定义数据
 * @return 0继续遍历，非0停止遍历
 */
typedef int (*dlist_cb_t)(dnode_t *node, void *userData);

/**
 * @brief 数据比较回调函数类型
 * @param src_data 源数据
 * @param target_data 目标数据
 * @return 0表示相等，非0表示不等
 */
typedef int (*dlist_cmp_cb_t)(void *src_data, void *target_data);
//**************************** Interface Structs ****************************//
//---------------------------------------------------------------------------//
//******************************** Classes **********************************//
/**
 * @brief 双向链表结构
 * @details 使用静态内存池管理节点，避免动态内存分配
 */
typedef struct
{
    dnode_t nodePool[DLIST_MAX_NODES];  // 静态内存池
    dnode_t *freeList;                  // 空闲节点链表头
    dnode_t *head;                      // 使用链表头
    dnode_t *tail;                      // 使用链表尾
    uint32_t length;                    // 当前节点数
    uint32_t maxNodes;                  // 最大节点数
} dlist_t;
//******************************** Classes **********************************//
//---------------------------------------------------------------------------//
//**************************** Extern Variables *****************************//
//**************************** Extern Variables *****************************//
//---------------------------------------------------------------------------//
//******************************** 函数声明 ***********************************//
/**
 * @brief  初始化链表实例（必须先调用）
 * @param  list: 链表实例指针
 * @return 状态码
 */
dlist_status_t dListInit(dlist_t* list);

/**
 * @brief  清空链表（自动释放data，若传入析构函数）
 */
void dListClear(dlist_t* list);

/**
 * @brief  头部插入
 */
dlist_status_t dListInsertHead(dlist_t* list, void* data);

/**
 * @brief  尾部插入
 */
dlist_status_t dListInsertTail(dlist_t* list, void* data);

/**
 * @brief  按位置插入（0=头部）
 */
dlist_status_t dListInsertAt(dlist_t* list, uint32_t pos, void* data);

/**
 * @brief  删除指定节点
 */
dlist_status_t dListDeleteNode(dlist_t* list, dnode_t* node);

/**
 * @brief  按值删除（第一个匹配项）
 */
dlist_status_t dListDeleteByValue(dlist_t* list, void* target_data, dlist_cmp_cb_t cmp);

/**
 * @brief  按位置删除
 */
dlist_status_t dListDeleteAt(dlist_t* list, uint32_t pos);

/**
 * @brief  按值查找节点
 */
dnode_t* dListFindByValue(dlist_t* list, void* target_data, dlist_cmp_cb_t cmp);

/**
 * @brief  按位置查找节点
 */
dnode_t* dListFindByPosition(dlist_t* list, uint32_t pos);

/**
 * @brief  反转链表
 */
void dListReverse(dlist_t* list);

/**
 * @brief  正向遍历
 */
void dListForEachForward(dlist_t* list, dlist_cb_t cb, void* userData);

/**
 * @brief  反向遍历
 */
void dListForEachBackward(dlist_t* list, dlist_cb_t cb, void* userData);
//******************************** 函数声明 ***********************************//

#endif //RTOS_PROJECT_DLIST_H