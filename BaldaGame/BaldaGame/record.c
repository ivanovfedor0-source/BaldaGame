#define _CRT_SECURE_NO_WARNINGS
#include "record.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

static Record records[5];
static int currentDisplayDifficulty = 0;

const char* getFilename(int difficulty) {
    switch (difficulty) {
    case DIFFICULTY_EASY:   return "records_easy.bin";
    case DIFFICULTY_MEDIUM: return "records_medium.bin";
    case DIFFICULTY_HARD:   return "records_hard.bin";
    default: return "records_easy.bin";
    }
}

void initRecords(void) {
    for (int diff = 0; diff < 3; diff++) {
        loadRecords(diff);
    }
}

void loadRecords(int difficulty) {
    const char* filename = getFilename(difficulty);
    FILE* f = fopen(filename, "rb");

    if (f) {
        fread(records, sizeof(Record), 5, f);
        fclose(f);
    }
    else {
        for (int i = 0; i < 5; i++) {
            strcpy(records[i].name, "---");
            records[i].score = 0;
            strcpy(records[i].date, "");
            records[i].extra.intValue = 0;
        }
        saveRecords(difficulty);
    }
}

void saveRecords(int difficulty) {
    const char* filename = getFilename(difficulty);
    FILE* f = fopen(filename, "wb");
    if (f) {
        fwrite(records, sizeof(Record), 5, f);
        fclose(f);
    }
}

int isHighScore(int score, int difficulty) {
    loadRecords(difficulty);
    for (int i = 0; i < 5; i++) {
        if (score > records[i].score) return 1;
    }
    return 0;
}

void addRecord(const char* name, int score, int difficulty) {
    loadRecords(difficulty);

    int existingIndex = -1;
    for (int i = 0; i < 5; i++) {
        if (strcmp(records[i].name, name) == 0) {
            existingIndex = i;
            break;
        }
    }
    if (existingIndex != -1) {
        if (score > records[existingIndex].score) {
            records[existingIndex].score = score;
            time_t t = time(NULL);
            struct tm* tm = localtime(&t);
            sprintf(records[existingIndex].date, "%02d.%02d.%04d",
                tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);

            for (int i = 0; i < 4; i++) {
                for (int j = i + 1; j < 5; j++) {
                    if (records[i].score < records[j].score) {
                        Record temp = records[i];
                        records[i] = records[j];
                        records[j] = temp;
                    }
                }
            }

            saveRecords(difficulty);
            printf("Record updated for %s! New score: %d\n", name, score);
        }
        else {
            printf("Record for %s already exists with score %d (not improved)\n", name, records[existingIndex].score);
        }
        return;
    }
    int worstScore = records[4].score;
    if (score <= worstScore) {
        printf("Score %d is not enough for top 5 (worst score is %d)\n", score, worstScore);
        return;
    }

    for (int i = 0; i < 5; i++) {
        if (score > records[i].score) {
            for (int j = 4; j > i; j--) {
                records[j] = records[j - 1];
            }

            strcpy(records[i].name, name);
            records[i].score = score;

            time_t t = time(NULL);
            struct tm* tm = localtime(&t);
            sprintf(records[i].date, "%02d.%02d.%04d",
                tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900);
            break;
        }
    }

    saveRecords(difficulty);
    printf("New record added for %s! Score: %d\n", name, score);
}

Record* getCurrentRecords(void) {
    return records;
}

void setRecordsDifficulty(int diff) {
    if (diff >= 0 && diff <= 2) {
        currentDisplayDifficulty = diff;
    }
}

int getRecordsDifficulty(void) {
    return currentDisplayDifficulty;
}

void loadRecordsForDisplay(void) {
    loadRecords(currentDisplayDifficulty);
}