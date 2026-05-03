#ifndef RECORD_H
#define RECORD_H

#include "menu.h"  // дл€ BotDifficulty

typedef struct {
    char name[20];
    int score;
    char date[20];
} Record;

// »нициализаци€ таблицы рекордов дл€ выбранной сложности
void initRecords(void);

// «агрузка рекордов дл€ указанной сложности
void loadRecords(int difficulty);

// —охранение рекордов дл€ указанной сложности
void saveRecords(int difficulty);

// ѕроверка, попадает ли результат в топ-5
int isHighScore(int score, int difficulty);

// ƒобавление нового рекорда
void addRecord(const char* name, int score, int difficulty);

// ѕолучение текущих рекордов (дл€ отрисовки)
Record* getCurrentRecords(void);

// ”становка текущей отображаемой сложности
void setRecordsDifficulty(int diff);

// ѕолучение текущей отображаемой сложности
int getRecordsDifficulty(void);

// «агрузка рекордов дл€ отображени€ (обновл€ет currentRecords)
void loadRecordsForDisplay(void);

#endif