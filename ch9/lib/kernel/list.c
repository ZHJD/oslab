#include "list.h"
#include "interrupt.h"

/*************************************
 * 函数名:list_init()
 * _list:list指针
 * 功能:初始化list
 * 返回值:无
 */
void list_init(list* _list)
{
    _list->head->prev = NULL;
    _list->head->next = &(_list->tail);
    _list->tail->prev = &(_list->head);
    _list->tail->next = NULL;
}

/**************************************************
 * 函数名:list_insert_before()
 * before:list_elem
 * elem:list_elem
 * 功能:在before之前插入elem
 * 返回值:无
 */
void list_insert_before(list_elem* before, list_elem* elem)
{
    /* 关闭中断 */
    intr_status old_status = intr_disable()

    before->prev->next = elem;
    elem->prev = before->prev;
    elem->next = before;
    before->prev = elem;
    intr_set_status(old_status);
}

/**************************************************
 * 函数名:list_push_head()
 * pList:链表
 * elem:list_elem元素
 * 功能:把elem插入到list的队首
 */
void list_push_head(list* pList, list_elem* elem)
{
    list_insert_before(pList->head.next, elem);
}

/***********************************************
 * 函数名:list_push_back
 * pList:list指针
 * elem:list_elem元素
 * 功能:在pList队列末尾插入elem
 * 返回值:无
 */
void list_push_back(list* pList, list_elem* elem)
{
    list_insert_before(&pList->tail, elem);
}

/***************************************************
 * 函数名:list_remove()
 * pElem:
 * 功能:使元素pElem脱离链表
 * 返回值:无
 */
void list_remove(list_elem* pElem)
{
    /* 关中断 */
    intr_status old_status = intr_disable();

    pElem->prev->next = pElem->next;
    pElem->next->prev = pElem->prev;

    /* 恢复关中断之前的状态 */
    intr_set_status(old_status);
}

/****************************************
 * 函数名:list_pop_head()
 * pList:list
 * 功能:去除链表中的第一个元素，并返回
 * 返回值:指向链表第一个元素的指针
 */
list_elem* list_pop_head(list* pList)
{
    list_elem* elem = pList->head.next;
    list_remove(elem);
    return elem;
}

/*****************************************************
 * 函数名:elem_find()
 * pList:list
 * obj_elem:待查找元素
 * 功能:查找元素
 * 返回值:true or false
 */
bool elem_find(list* pList, list_elem* obj_elem)
{
    for(list_elem* elem = pList->head.next; elem != &pList->tail; elem = elem->next)
    {
        if(elem == obj_elem)
        {
            return true;
        }
    }
    return false;
}

/*******************************************
 * 函数名:list_traversal()
 * pList:list
 * func:自定义函数
 * arg:func参数
 * 功能:遍历列表中的每个元素elem和arg传给回调函数function
 * 判断是否符合条件。
 * 返回值:返回指向指定元素的指针或者NULL
 */ 
list_elem* list_traversal(list* pList, function func, int arg)
{
    list_elem* elem = pList->head.next;
    /* 如果队列为空，就必然没有符合条件的结点，故直接返回NULL */
    if(list_empty(pList))
    {
        return NULL;
    }
    while(elem != &pList->tail)
    {
        if(function(elem, arg))
        {
            return elem;
        }
        elem = elem->next;
    }
    return false;
}

/****************************************************
 * 函数名:list_len()
 * pList:list
 * 功能:list长度
 * 返回值:返回list的长度
 */ 
uint32_t list_len(list* pList)
{
    uint32_t len = 0;
    for(list_elem* elem = pList->head.next; elem != &pList->tail; elem = elem->next)
    {
        len++;
    }
    return len;
}

/*************************************************************
 * 函数名:list_empty()
 * pList:list
 * 功能:判断list是否为空
 * 返回值:如果为空，返回true,非空，返回false
 */ 
bool list_empty(list* pList)
{
    return (pList->head.next == &pList->tail);
}

