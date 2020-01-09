#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "json.h"

#define IS_Content_String		1
#define IS_Content_NOString		2
#define IS_Object				3
#define IS_Array				4
#define IS_Combination			5

static int decodeJsonCombinationHame(JsonMemoryPool** mem,JsonCombinationHame* combination,char* star);
static int decodeJsonObjectHame(JsonMemoryPool** mem,JsonObjectHame* object,char* star);
static int decodeJsonArrayHame(JsonMemoryPool** mem,JsonArrayHame* array,char* star);

static void showCombinationJson(JsonCombinationHame* Combination,int n);
static void showArrayJson(JsonArrayHame* Array,int n);
static void showObjectJson(JsonObjectHame* Object,int n);
static void showContentJson(JsonContentHame* Content,int n);

static int getErrorCode()
{
	return 0;
}

static void* MallocJsonMem(JsonMemoryPool** pool,int size)
{
	void* lp;
	JsonMemoryPool* data;
	if(!pool || !*pool)
	{
		return NULL;
	}
	data = *pool;
	if(data->len < size)
	{
		JsonMemoryPool* mem = (JsonMemoryPool*)malloc(sizeof(JsonMemoryPool));
		if(!mem)
		{
			printf("MallocJsonMem malloc:%d\n", getErrorCode());
			return NULL;			
		}
		memset(mem,0,sizeof(JsonMemoryPool));
		mem->len = MEMORY_POOL_JSON;
		mem->next = *pool;
		*pool = mem;
		data = mem;
		if(size>mem->len)
		{
			printf("MallocJsonMem size More than %d\n",MEMORY_POOL_JSON);
			return NULL;
		}
	}
	data->len -= size;
	lp = (void*)(data->data + data->offset);
	data->offset += size;
	return lp;		
}

static int FreeJsonMem(JsonMemoryPool** pool)
{
	JsonMemoryPool* data;
	JsonMemoryPool* last;
	if(!pool)
	{
		return -1;
	}	
	data = *pool;
	while(data)
	{
		last = data->next;
		free(data);
		data = last;
	}
	*pool = NULL;
	return 0;
}

static int GetElementType(char* star,int* len)
{
	int i;
	*len = 0;
	for(i=0;star[i] && star[i]!=':';i++);
	if(!star[i])
	{
		printf("GetElementType not find :\n");
		return -1;		
	}
	
	i++;
	while(star[i])
	{
		if(star[i] == '{')
		{
			*len = i+1;
			return IS_Object;
		}else if(star[i] == '[')
		{
			*len = i+1;
			return IS_Array;
		}else if(star[i] == '"')
		{
			*len = i;
			return IS_Content_String;
		}else if((star[i]>='0' && star[i]<='9') ||
		(star[i]>='a' && star[i]<='z') ||
		(star[i]>='A' && star[i]<='Z') ||
		(star[i]=='-' && star[i+1]>='0' && star[i+1]<='9'))
		{
			*len = i;
			return IS_Content_NOString;
		}
		i++;
	}
	return -1;	
}

static int IsOverJson(char* star,int c,int* len)
{
	int i = 0;
	*len = 0;
	while(star[i])
	{
		if(star[i]==c)
		{
			*len = i+1;
			return 1;
		}else if(star[i]==' ' || star[i]=='\r' || star[i]=='\n' || star[i]=='\t')
		{
			i++;
		}else
		{
			return 0;
		}
	}
	return 0;	
}

static char* GetIntFromJson(char* star,int* len,int* isOver)
{
	char* value = NULL;
	int i;
	*len = 0;
	*isOver = 0;
	for(i=0;star[i] && star[i]!=',' && star[i]!='}';i++)
	{
		if(!value)
		{
			if((star[i]>='0' && star[i]<='9') ||
			(star[i]>='a' && star[i]<='z') ||
			(star[i]>='A' && star[i]<='Z') || 
			(star[i]=='-' && i==0) || 
			(star[i]=='.' && star[i+1]>='0' && star[i+1]<='9'))
			{
				value = star+i;	
			}	
		}else
		{
			if(star[i]==' ' || star[i]=='\r' || star[i]=='\n' || star[i]=='\t')
			{
				star[i] = '\0';
			}
		}
	}
	if(!value)
	{
		return NULL;
	}
	if(star[i]=='}')
	{
		*isOver = 1;	
	}
	star[i] = '\0';
	*len = i+1;
	return value;	
}

static char* GetStringFromJson(char* star,int* len)
{
	char* value = "";
	int i;
	*len = 0;
	for(i=0;star[i] && star[i]!='"';i++);
	if(!star[i])
	{
		printf("GetStringFromJson not find \"\n");
		return NULL;		
	}

	i++;
	value = star+i;
	if(!star[i])
	{
		printf("GetStringFromJson not find \"\n");
		return NULL;		
	}	
	
	while(star[i])
	{
		if(star[i]=='"' && star[i-1]!='\\')
		{
			star[i] = '\0';
			*len = i+1;	
			return value;	
		}
		i++;
	}
	
	printf("GetStringFromJson not find \"\n");
	return NULL;			
}

/*组合*/
static int decodeJsonCombinationHame(JsonMemoryPool** mem,JsonCombinationHame* combination,char* star)
{
	int len,type,ret;
	int isOver = 0;
	char* str;
	char* ptr;
	char* bufr_star = star;
	while(!isOver)
	{
		if(IsOverJson(star,'}',&len))
		{
			star += len;	
			break;
		}

		str = GetStringFromJson(star,&len);
		if(!str)
		{
			printf("GetStringFromJson error");
			return -1;
		}

		star += len;

		type = GetElementType(star,&len);
		if(type<0)
		{
			printf("GetElementType error\n");
			return -1;		
		}
		
		if(IS_Content_String==type)
		{
			JsonContentHame* data;
			star += len;
			
			ptr = GetStringFromJson(star,&len);
			if(!ptr)
			{
				printf("GetStringFromJson error\n");
				return -1;
			}
			
			star += len;

			data = (JsonContentHame*)MallocJsonMem(mem,sizeof(JsonContentHame));
			if(!data)
			{
				printf("GetStringFromJson error\n");
				return -1;				
			}
			
			data->name = str;
			data->value = ptr;
			data->is_string = 1;
					
			if(combination->Content)
			{
				JsonContentHame* last = NULL;
				JsonContentHame* node = combination->Content;
				while(node)
				{
					last = node;
					node = node->next;
				}
				last->next = data;
			}else
			{
				combination->Content = data;			
			}
		}else if(IS_Content_NOString==type)
		{
			JsonContentHame* data;
			star += len;

			ptr = GetIntFromJson(star,&len,&isOver);
			if(!ptr)
			{
				printf("GetStringFromJson error\n");
				return -1;
			}
			star += len;
			
			data = (JsonContentHame*)MallocJsonMem(mem,sizeof(JsonContentHame));
			if(!data)
			{
				printf("MallocJsonMem error\n");
				return -1;
			}			

			data->name = str;
			data->value = ptr;
							
			if(combination->Content)
			{
				JsonContentHame* last = NULL;
				JsonContentHame* node = combination->Content;
				while(node)
				{
					last = node;
					node = node->next;
				}

				last->next = data;
			}else
			{		
				combination->Content = data;		
			}			
		}else if(IS_Object==type)
		{
			JsonObjectHame* data = (JsonObjectHame*)MallocJsonMem(mem,sizeof(JsonObjectHame));
			if(!data)
			{
				printf("MallocJsonMem error\n");
				return -1;
			}
			
			data->name = str;
			
			if(combination->Object)
			{
				JsonObjectHame* last = NULL;
				JsonObjectHame* node = combination->Object;
				while(node)
				{
					last = node;
					node = node->next;
				}
				last->next = data;
			}else
			{
				combination->Object = data;
			}
			
			star += len;
			ret = decodeJsonObjectHame(mem,data,star);
			if(ret<0)
			{
				printf("decodeJsonObjectHame error\n");
				return -1;
			}
			star += ret;
		}else if(IS_Array==type)
		{
			JsonArrayHame* data = (JsonArrayHame*)MallocJsonMem(mem,sizeof(JsonArrayHame));
			if(!data)
			{
				printf("MallocJsonMem error\n");
				return -1;
			}
			
			data->name = str;
			
			if(combination->Array)
			{
				JsonArrayHame* last = NULL;
				JsonArrayHame* node = combination->Array;
				while(node)
				{
					last = node;
					node = node->next;	
				}
				last->next = data;
			}else
			{
				combination->Array = data;
			}
			
			star += len;
			ret = decodeJsonArrayHame(mem,data,star);
			if(ret<0)
			{
				printf("decodeJsonArrayHame error\n");
				return -1;
			}
			star += ret;			
		}
	
	}
	return star - bufr_star;
}

/*对象*/
static int decodeJsonObjectHame(JsonMemoryPool** mem,JsonObjectHame* object,char* star)
{
	int len,type,ret;
	int isOver = 0;
	char* str;
	char* ptr;
	char* bufr_star = star;
	while(!isOver)
	{
		if(IsOverJson(star,'}',&len))
		{
			star += len;
			break;
		}
				
		str = GetStringFromJson(star,&len);
		if(!str)
		{
			printf("GetStringFromJson error\n");
			return -1;
		}
		
		star += len;	
		
		type = GetElementType(star,&len);
		if(type<0)
		{
			printf("GetElementType error\n");
			return -1;		
		}
		
		if(IS_Content_String==type)
		{
			JsonContentHame* data;
			star += len;
			
			ptr = GetStringFromJson(star,&len);
			if(!ptr)
			{
				printf("GetStringFromJson error\n");
				return -1;
			}
			
			star += len;

			data = (JsonContentHame*)MallocJsonMem(mem,sizeof(JsonContentHame));
			if(!data)
			{
				printf("GetStringFromJson error\n");
				return -1;				
			}
			
			data->name = str;
			data->value = ptr;
			data->is_string = 1;
					
			if(object->Content)
			{
				JsonContentHame* last = NULL;
				JsonContentHame* node = object->Content;
				while(node)
				{
					last = node;
					node = node->next;
				}
				last->next = data;
			}else
			{
				object->Content = data;			
			}
		}else if(IS_Content_NOString==type)
		{
			JsonContentHame* data;
			star += len;
			
			ptr = GetIntFromJson(star,&len,&isOver);
			if(!ptr)
			{
				printf("GetStringFromJson error\n");
				return -1;
			}
			star += len;
			
			data = (JsonContentHame*)MallocJsonMem(mem,sizeof(JsonContentHame));
			if(!data)
			{
				printf("MallocJsonMem error\n");
				return -1;
			}			

			data->name = str;
			data->value = ptr;
							
			if(object->Content)
			{
				JsonContentHame* last = NULL;
				JsonContentHame* node = object->Content;
				while(node)
				{
					last = node;
					node = node->next;
				}

				last->next = data;
			}else
			{		
				object->Content = data;		
			}			
		}else if(IS_Object==type)
		{
			JsonObjectHame* data = (JsonObjectHame*)MallocJsonMem(mem,sizeof(JsonObjectHame));
			if(!data)
			{
				printf("MallocJsonMem error\n");
				return -1;
			}
			
			data->name = str;
			
			if(object->Object)
			{
				JsonObjectHame* last = NULL;
				JsonObjectHame* node = object->Object;
				while(node)
				{
					last = node;
					node = node->next;
				}
				last->next = data;
			}else
			{
				object->Object = data;
			}
			
			star += len;
			ret = decodeJsonObjectHame(mem,data,star);
			if(ret<0)
			{
				printf("decodeJsonObjectHame error\n");
				return -1;
			}
			star += ret;
		}else if(IS_Array==type)
		{
			JsonArrayHame* data = (JsonArrayHame*)MallocJsonMem(mem,sizeof(JsonArrayHame));
			if(!data)
			{
				printf("MallocJsonMem error\n");
				return -1;
			}
			
			data->name = str;
			
			if(object->Array)
			{
				JsonArrayHame* last = NULL;
				JsonArrayHame* node = object->Array;
				while(node)
				{
					last = node;
					node = node->next;	
				}
				last->next = data;
			}else
			{
				object->Array = data;
			}
			
			star += len;
			ret = decodeJsonArrayHame(mem,data,star);
			if(ret<0)
			{
				printf("decodeJsonArrayHame error\n");
				return -1;
			}
			star += ret;			
		}
	
	}
	return star - bufr_star;		
}

/*数组*/
static int decodeJsonArrayHame(JsonMemoryPool** mem,JsonArrayHame* array,char* star)
{	
	int len,ret;
	char* bufr_star = star;
	JsonCombinationHame* Combination;
	while(1)
	{		
		if(IsOverJson(star,']',&len))
		{
			star += len;	
			break;
		}
					
		Combination = (JsonCombinationHame*)MallocJsonMem(mem,sizeof(JsonCombinationHame));
		if(!Combination)
		{
			printf("MallocJsonMem error\n");
			return -1;
		}
			
		if(array->Combination)
		{
			JsonCombinationHame* last = NULL;
			JsonCombinationHame* node = array->Combination;
			while(node)
			{
				last = node;
				node = node->next;
			}
			last->next = Combination;
		}else
		{
			array->Combination = Combination;
		}
				
		ret = decodeJsonCombinationHame(mem,Combination,star);
		if(ret<0)
		{
			printf("decodeJsonCombinationHame error\n");
			return -1;
		}
		
		array->CombinationNum++;
				
		star += ret;		
	}
	return star - bufr_star;	
}


JsonHandlHame* initJosnHame(char* buffer)
{
	char* star = buffer;
	char* end = NULL;
	char* data = NULL;
	int size;
	JsonHandlHame* json;
	JsonCombinationHame* combination;
	JsonMemoryPool* mem;
	
	if(!buffer || !buffer[0])
	{
		printf("initJosnHame buffer=null\n");
		return NULL;
	}
	while(*star && *star!='{')star++;
	end = strrchr(star,'}');
	if(!end)
	{
		printf("initJosnHame buffer end not find }\n");
		return NULL;		
	}
	size = (end - star) + 2;
	
	data = (char*)malloc(size);
	if(!data)
	{
		printf("initJosnHame malloc:%d\n", getErrorCode());
		return NULL;
	}
	memcpy(data,star,size-1);
	data[size-1] = '\0';
	
	json = (JsonHandlHame*)malloc(sizeof(JsonHandlHame));
	if(!json)
	{
		printf("initJosnHame malloc:%d\n", getErrorCode());
		free(data);
		return NULL;		
	}
	memset(json,0,sizeof(JsonHandlHame));
	
	mem = (JsonMemoryPool*)malloc(sizeof(JsonMemoryPool));
	if(!mem)
	{
		printf("initJosnHame malloc:%d\n", getErrorCode());
		free(data);
		free(json);
		return NULL;		
	}
	memset(mem,0,sizeof(JsonMemoryPool));
	mem->len = MEMORY_POOL_JSON;
	
	json->data = data;
	json->mem = mem;
	combination = (JsonCombinationHame*)MallocJsonMem(&json->mem,sizeof(JsonCombinationHame));
	if(!combination)
	{
		printf("MallocJsonMem error\n");
		FreeJsonMem(&json->mem);
		free(data);
		free(json);		
		return NULL;	
	}
	
	json->Combination = combination;
	
	if(decodeJsonCombinationHame(&json->mem,combination,data)<0)
	{
		freeJsonHame(json);
		return NULL;
	}
	return json;
}

int freeJsonHame(JsonHandlHame* json)
{
	if(json)
	{
		if(json->data)
		{
			free(json->data);
		}
		if(json->mem)
		{
			JsonMemoryPool* mem = json->mem;
			JsonMemoryPool* last;
			while(mem)
			{
				last = mem->next;
				free(mem);
				mem = last;
			}
		}
		free(json);
	}	
	return 0;
}

static void showEmptyFill(int n)
{
	while(n-->0)
	{
		printf("  ");
	}	
}

static void showContentJson(JsonContentHame* Content,int n)
{
	while(Content)
	{
		showEmptyFill(n);
		printf("%s : %s\n",Content->name,Content->value);
		Content = Content->next;
	}	
}

static void showObjectJson(JsonObjectHame* Object,int n)
{
	while(Object)
	{
		showEmptyFill(n);
		printf("%s :\n",Object->name);
		
		showEmptyFill(n);
		printf("{\n");
		
		showContentJson(Object->Content,n+1);
		
		showArrayJson(Object->Array,n+1);
		
		showObjectJson(Object->Object,n+1);
	 	
		showEmptyFill(n);
		printf("}\n");		
		
		Object = Object->next;
	}
}

static void showArrayJson(JsonArrayHame* Array,int n)
{
	while(Array)
	{
		showEmptyFill(n);
		printf("%s :\n",Array->name);
		
		showEmptyFill(n);
		printf("[\n");
				
		showCombinationJson(Array->Combination,n+1);
		
		showEmptyFill(n);
		printf("]\n");		
		
		Array = Array->next;
	}
}

static void showCombinationJson(JsonCombinationHame* Combination,int n)
{
	while(Combination)
	{
		showEmptyFill(n);
		printf("{\n");
		
		showContentJson(Combination->Content,n+1);
		
		showArrayJson(Combination->Array,n+1);
		
		showObjectJson(Combination->Object,n+1);
		
		showEmptyFill(n);		
		printf("}\n");
		Combination = Combination->next;
	}
}

void showJsonHame(JsonHandlHame* json)
{
	if(json)
	{
		showCombinationJson(json->Combination,0);
	}
}

#if MAIN_TEST
int main(int argc,char** argv)
{
	FILE* fp;
	JsonHandlHame* json;
	char bufr[1024*256];
	
	memset(bufr,0,sizeof(bufr));
	printf("Input JSON file: \b");
	fgets(bufr,2048,stdin);
	if(bufr[0])
	{
		bufr[strlen(bufr)-1] = '\0';
	}
	
	fp = fopen(bufr,"r");
	if(!fp)
	{
		perror("fopen");
		return 0;
	}
	
	memset(bufr,0,sizeof(bufr));
	fread(bufr,1,sizeof(bufr),fp);
	fclose(fp);
	
//	printf("%s\n",bufr);


	json = initJosnHame(bufr);
	if(!json)
	{
		printf("initJosnHame error\n");
	}
	
	showJsonHame(json);
	
	freeJsonHame(json);
	
	fgets(bufr,1024,stdin);
	return 0;
}
#endif
