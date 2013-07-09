/*
 * @file:conffile.c
 * @brief initialization file read and write API implementation
 * @author WuMin
 * @date 2012-8-26
 * @version 0.1
*/

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_READ_LINE_NUM 		1024
#define MAX_ITEM_COMMENT_NUM	50
#define MAX_ITEMS_NUM			100

typedef struct _item_t_ {
	char **comment;
	short num;
	char *key;
	char *value;
} item_t;

/* 去除字符串右端空格 */
char *strtrimr(char *pstr)
{
	int i;

	i = strlen(pstr) - 1;
	while (isspace(pstr[i]) && (i >= 0))
		pstr[i--] = '\0';

	return pstr;
}

/* 去除字符串左端空格 */
char *strtriml(char *pstr)
{
	int i = 0, j;

	j = strlen(pstr) - 1;
	while (isspace(pstr[i]) && (i <= j))
		i++;
	if (0 < i)
		strcpy(pstr, &pstr[i]);

	return pstr;
}

/* 去除字符串两端空格 */
char *strtrim(char *pstr)
{
	char *p;

	p = strtrimr(pstr);

	return strtriml(p);
}

void free_item_space(item_t * item)
{
	int i;

	if (item == NULL)
		return;

	if (item->comment != NULL) {
		for (i = 0; i < item->num; i++) {
			free(item->comment[i]);
			item->comment[i] = NULL;
		}

		free(item->comment);
		item->comment = NULL;
	}

	item->num = 0;

	if (item->key != NULL) {
		free(item->key);
		item->key = NULL;
	}

	if (item->value != NULL) {
		free(item->value);
		item->value = NULL;
	}

	return;
}

/* 从配置文件读出key或value,数据返回到item_t指针 */
int get_item_from_line(char *line, item_t * item)
{
	char *p = NULL;
	char *p2 = NULL;
	int len = 0;

	p = strtrim(line);
	len = strlen(p);

	if (len <= 0) {
		return -1;
	} else if (p[0] == '#') {
		if (item->num > MAX_ITEM_COMMENT_NUM)
			return 1;

		if (item->comment == NULL) {
			item->comment =
				(char **)malloc(MAX_ITEM_COMMENT_NUM * sizeof(char *));
			if (item->comment == NULL) {
				printf("malloc error!\n");
				return -1;
			}
		}

		item->comment[item->num] = (char *)malloc(len + 1);
		strcpy(item->comment[item->num], p);
		item->num++;
		return 1;
	} else {
		p2 = strchr(p, '=');
		if (p2 == NULL) {
			p2 = strchr(p, ' ');
			if (p2 != NULL) {
				*p2++ = '\0';
			} else {
				return -1;
			}
		} else {
			*p2++ = '\0';
		}

		item->key = (char *)malloc(strlen(p) + 1);
		item->value = (char *)malloc(strlen(p2) + 1);
		strcpy(item->key, p);
		strcpy(item->value, p2);
	}

	return 0;
}

int file_to_memory(const char *file, item_t * items, int items_num)
{
	FILE *fp = NULL;
	int i = 0, j = 0;
	char line[MAX_READ_LINE_NUM];

	fp = fopen(file, "r");
	if (fp == NULL) {
		printf("file %s not exit!\n", file);
		return -1;
	}

	memset(items, 0, (items_num * sizeof(item_t)));

	while (fgets(line, MAX_READ_LINE_NUM - 1, fp)) {
		if (i >= MAX_ITEMS_NUM) {
			printf("too much items,only support 100 items!\n");
			return MAX_ITEMS_NUM;
		}

		if (!get_item_from_line(line, &items[i])) {
			i++;
		}
	}

	for (j < 0; j < i + 1; j++) {
		if (items[j].comment == NULL && items[j].key == NULL
			&& items[j].value == NULL)
			break;
	}

	fclose(fp);
	fp = NULL;

	return j;
}

int read_conf_value(const char *key, char *value, const char *file)
{
	FILE *fp = NULL;
	item_t item;
	char line[MAX_READ_LINE_NUM];

	if (key == NULL || value == NULL || file == NULL) {
		printf("params error!\n");
		return -1;
	}

	fp = fopen(file, "r");
	if (fp == NULL) {
		printf("file %s not exit!\n", file);
		return -1;
	}

	memset(&item, 0, sizeof(item_t));

	while (fgets(line, MAX_READ_LINE_NUM - 1, fp)) {
		if (!get_item_from_line(line, &item)) {
			if (item.key != NULL && item.value != NULL) {
				if (!strcmp(item.key, key)) {
					strcpy(value, item.value);
					break;
				}
			}
			free_item_space(&item);
		}
	}

	free_item_space(&item);
	fclose(fp);
	fp = NULL;

	return 0;
}

int write_conf_value(const char *key, char *value, const char *file)
{
	int i = 0, j = 0;
	int num = 0;
	FILE *fp = NULL;
	item_t items[MAX_ITEMS_NUM];
	int flag = 0;

	if (key == NULL || value == NULL || file == NULL) {
		printf("params error!\n");
		return -1;
	}

	num = file_to_memory(file, items, MAX_ITEMS_NUM);
	if (num < 0) {
		return -1;
	}

	for (i = 0; i < num; i++) {
		if (items[i].key != NULL && items[i].value != NULL) {
			if (!strcmp(items[i].key, key)) {
				strcpy(items[i].value, value);
				flag = 1;
				break;
			}
		}
	}

	fp = fopen(file, "w");
	if (fp == NULL) {
		return -1;
	}

	for (i = 0; i < num; i++) {
		if (items[i].comment != NULL) {
			fprintf(fp, "\n");
			for (j = 0; j < items[i].num; j++) {
				if (items[i].comment[j] != NULL)
					fprintf(fp, "%s\n", items[i].comment[j]);
			}
			fprintf(fp, "\n");
		}

		if (items[i].key != NULL && items[i].value != NULL)
			fprintf(fp, "%s=%s\n", items[i].key, items[i].value);
	}

	if (flag == 0) {
		fprintf(fp, "%s=%s\n", key, value);
	}

	for (i = 0; i < num; i++) {
		free_item_space(&items[i]);
	}

	fclose(fp);
	fp = NULL;

	return 0;
}
