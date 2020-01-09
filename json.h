#ifndef _JSON_H___H
#define _JSON_H___H

#define MEMORY_POOL_JSON	4096

/*���� �� ֵ*/
typedef struct JsonContentHame
{
	char* name;
	char* value;
	int is_string;
	struct JsonContentHame* next;
}JsonContentHame;

/*����*/
typedef struct JsonObjectHame
{
	char* name;
	struct JsonContentHame* Content;
	struct JsonArrayHame* Array;
	struct JsonObjectHame* Object;
	struct JsonObjectHame* next;
}JsonObjectHame;

/*����*/
typedef struct JsonArrayHame
{
	char* name;
	int CombinationNum;
	struct JsonCombinationHame* Combination;
	struct JsonArrayHame* next;
}JsonArrayHame;

/*��� �Ͷ����������û�� ���� */
typedef struct JsonCombinationHame
{
	struct JsonContentHame* Content;
	struct JsonArrayHame* Array;
	struct JsonObjectHame* Object;
	struct JsonCombinationHame* next;
}JsonCombinationHame;

/*�ڴ��*/
typedef struct JsonMemoryPool
{
	int offset;
	int len;
	struct JsonMemoryPool* next;
	char data[MEMORY_POOL_JSON];
}JsonMemoryPool;

/*JSON ���*/
typedef struct JsonHandlHame
{
	char* data;
	struct JsonMemoryPool* mem;
	struct JsonCombinationHame* Combination;	
}JsonHandlHame;

/*
���� ��ʼ�� JSON ���� 
���� JSON ���  
*/
JsonHandlHame* initJosnHame(char* buffer);

/*
���� �г�JSON ����  
*/
void showJsonHame(JsonHandlHame* json);

/*
���� �����ڴ� 
*/
int freeJsonHame(JsonHandlHame* json);

#endif
