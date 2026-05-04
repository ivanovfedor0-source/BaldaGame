#ifndef RECORD_H
#define RECORD_H

#include "menu.h"

union RecordExtra {
    int intValue;
    float floatValue;
    char stringValue[20];
};

typedef struct {
    char name[20];
    int score;
    char date[20];
    union RecordExtra extra;
} Record;

void initRecords(void);

void loadRecords(int difficulty);

void saveRecords(int difficulty);

int isHighScore(int score, int difficulty);

void addRecord(const char* name, int score, int difficulty);

Record* getCurrentRecords(void);

void setRecordsDifficulty(int diff);

int getRecordsDifficulty(void);

void loadRecordsForDisplay(void);

#endif