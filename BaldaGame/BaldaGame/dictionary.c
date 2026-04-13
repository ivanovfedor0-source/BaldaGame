#define _CRT_SECURE_NO_WARNINGS
#include "dictionary.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static struct DictNode* root = NULL;

struct DictNode* insertWord(struct DictNode* node, const char* word) {
    if (node == NULL) {
        node = (struct DictNode*)malloc(sizeof(struct DictNode));
        strcpy(node->word, word);
        node->left = NULL;
        node->right = NULL;
        return node;
    }

    int cmp = strcmp(word, node->word);
    if (cmp < 0) {
        node->left = insertWord(node->left, word);
    }
    else if (cmp > 0) {
        node->right = insertWord(node->right, word);
    }
    return node;
}

int searchWord(struct DictNode* node, const char* word) {
    if (node == NULL) return 0;

    int cmp = strcmp(word, node->word);
    if (cmp == 0) return 1;
    if (cmp < 0) return searchWord(node->left, word);
    return searchWord(node->right, word);
}

void freeTree(struct DictNode* node) {
    if (node == NULL) return;
    freeTree(node->left);
    freeTree(node->right);
    free(node);
}

void loadDictionary(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Warning: Cannot open dictionary file %s\n", filename);
        return;
    }

    char word[100];
    int count = 0;

    while (fgets(word, sizeof(word), file)) {
        word[strcspn(word, "\n")] = '\0';
        if (strlen(word) == 0) continue;

        int len = strlen(word);
        if (len >= 3 && len <= 8) {
            root = insertWord(root, word);
            count++;
        }
    }

    fclose(file);
    printf("Dictionary loaded: %d words\n", count);
}

int isWordInDictionary(const char* word) {
    if (root == NULL) return 0;
    return searchWord(root, word);
}

void freeDictionary() {
    freeTree(root);
    root = NULL;
}