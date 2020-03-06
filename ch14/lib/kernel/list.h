#ifndef __LIB_KERNEL_LIST_H
#define __LIB_KERNEL_LIST_H

#include "global.h"

/* 获取结构体成员在结构体中的偏移量 */
#define offset(struct_type, member) (uint32_t)(&((struct_type*)0)->member)

/* 由结构体成员地址得到结构体变量起始地址 */
#define elem2entry(struct_type, struct_member_name, elem_ptr) \
        (struct_type*)((uint32_t)elem_ptr - offset(struct_type, struct_member_name))



/********** 定义链表结点成员结构 **************
 * 结点中不需要数据成员，只要求前驱和后继结点指针
 */
typedef struct list_elem
{
    /* 前驱结点 */
    struct list_elem* prev;

    /* 后驱结点 */
    struct list_elem* next;
}list_elem;

/* 链表结构，用于实现队列 */
typedef struct list
{
    /* head是队首，固定不变，第一个元素是head.next */
    list_elem head;

    /* tail是队尾，固定不变  */
    list_elem tail;
}list;

/* 自定义函数类型function,用于在list_traversal中做回调函数 */
typedef bool function(list_elem*, int arg);


/*************************************
 * 函数名:list_init()
 * _list:list指针
 * 功能:初始化list
 * 返回值:无
 */
void list_init(list* _list);

/**************************************************
 * 函数名:list_insert_before()
 * before:list_elem
 * elem:list_elem
 * 功能:在before之前插入elem
 * 返回值:无
 */
void list_insert_before(list_elem* before, list_elem* elem);

/**************************************************
 * 函数名:list_push_head()
 * pList:链表
 * elem:list_elem元素
 * 功能:把elem插入到list的队首
 */
void list_push_head(list* pList, list_elem* elem);

/***********************************************
 * 函数名:list_push_back
 * pList:list指针
 * elem:list_elem元素
 * 功能:在pList队列末尾插入elem
 * 返回值:无
 */
void list_push_back(list* pList, list_elem* elem);

/***************************************************
 * 函数名:list_remove()
 * pElem:
 * 功能:使元素pElem脱离链表
 * 返回值:无
 */
void list_remove(list_elem* pElem);

/****************************************
 * 函数名:list_pop_head()
 * pList:list
 * 功能:去除链表中的第一个元素，并返回
 * 返回值:指向链表第一个元素的指针
 */
list_elem* list_pop_head(list* pList);

/*****************************************************
 * 函数名:elem_find()
 * pList:list
 * obj_elem:待查找元素
 * 功能:查找元素
 * 返回值:true or false
 */
bool elem_find(list* pList, list_elem* obj_elem);

/*******************************************
 * 函数名:list_traversal()
 * pList:list
 * func:自定义函数
 * arg:func参数
 * 功能:遍历列表中的每个元素elem和arg传给回调函数function
 * 判断是否符合条件。
 * 返回值:返回指向指定元素的指针或者NULL
 */ 
list_elem* list_traversal(list* pList, function* func, int arg);

/****************************************************
 * 函数名:list_len()
 * pList:list
 * 功能:list长度
 * 返回值:返回list的长度
 */ 
uint32_t list_len(list* pList);

/*************************************************************
 * 函数名:list_empty()
 * pList:list
 * 功能:判断list是否为空
 * 返回值:如果为空，返回true,非空，返回false
 */ 
bool list_empty(list* pList);










#endif
