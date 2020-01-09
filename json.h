#ifndef _JSON_H___H
#define _JSON_H___H

#define MEMORY_POOL_JSON	4096

/*名称 与 值*/
typedef struct JsonContentHame
{
	char* name;
	char* value;
	int is_string;
	struct JsonContentHame* next;
}JsonContentHame;

/*对象*/
typedef struct JsonObjectHame
{
	char* name;
	struct JsonContentHame* Content;
	struct JsonArrayHame* Array;
	struct JsonObjectHame* Object;
	struct JsonObjectHame* next;
}JsonObjectHame;

/*数组*/
typedef struct JsonArrayHame
{
	char* name;
	int CombinationNum;
	struct JsonCombinationHame* Combination;
	struct JsonArrayHame* next;
}JsonArrayHame;

/*组合 和对象的区别是没有 名称 */
typedef struct JsonCombinationHame
{
	struct JsonContentHame* Content;
	struct JsonArrayHame* Array;
	struct JsonObjectHame* Object;
	struct JsonCombinationHame* next;
}JsonCombinationHame;

/*内存池*/
typedef struct JsonMemoryPool
{
	int offset;
	int len;
	struct JsonMemoryPool* next;
	char data[MEMORY_POOL_JSON];
}JsonMemoryPool;

/*JSON 句柄*/
typedef struct JsonHandlHame
{
	char* data;
	struct JsonMemoryPool* mem;
	struct JsonCombinationHame* Combination;	
}JsonHandlHame;

/*
功能 初始化 JSON 数据 
返回 JSON 句柄  
*/
JsonHandlHame* initJosnHame(char* buffer);

/*
功能 列出JSON 数据  
*/
void showJsonHame(JsonHandlHame* json);

/*
功能 清零内存 
*/
int freeJsonHame(JsonHandlHame* json);

#endif
