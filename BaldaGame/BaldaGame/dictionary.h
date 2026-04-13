#ifndef DICTIONARY_H
#define DICTIONARY_H

struct DictNode {
    char word[30];
    struct DictNode* left;
    struct DictNode* right;
};


void loadDictionary(const char* filename);
int isWordInDictionary(const char* word);
void freeDictionary();

#endif