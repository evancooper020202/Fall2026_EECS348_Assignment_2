/*******************************************************************************
 * Program Name: EECS 348 Assignment - Email Priority Queue Manager
 *
 * Description:
 *      A C program that manages and prioritizes incoming emails for a CEO using
 *      a list-based MaxHeap priority queue. Emails are prioritized first by sender 
 *      category tier (Boss > Subordinate/Peer/ImportantPerson > OtherPerson), 
 *      second by date (newest first), and third by order of arrival.
 *
 * Inputs:
 *      Command stream via standard input (stdin) containing formatted lines:
 *      - EMAIL <sender category>, <subject line>, <date>
 *      - NEXT  (Inspect highest-priority email)
 *      - READ  (Dequeue/process highest-priority email)
 *      - COUNT (Output unread email count)
 *
 * Output:
 *      Formatted text displayed to terminal based on commands processed.
 *
 * Collaborators: None
 * Other Sources: ChatGPT / AI Assistance
 *
 * Author: Evan Cooper
 * Creation Date: September 17, 2026
 * Revision Date: September 17, 2026
 * Revisions:
 *      - Fixed priority tier bug to group Subordinate, Peer, ImportantPerson equally.
 *      - Pre-parsed YYYYMMDD integer date into struct to avoid log(N) sscanf calls.
 *      - Replaced hardcoded category rankings with explicit enumeration types.
 *      - Optimized struct sizes to improve memory layout and cache locality.
 * - most of the code was written by ChatGPT including fixes and adjustments but I wrote comments
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10

// Category tiers defined by priority status
typedef enum {
    PRIORITY_OTHER = 1, // OtherPerson - read last
    PRIORITY_MID   = 2, // Subordinate, Peer, ImportantPerson - read next
    PRIORITY_BOSS  = 3  // Boss - read first
} PriorityTier;

// Compact struct layout to optimize memory footprint & cache performance
typedef struct {
    char category[24];
    char subject[128];
    char date[12];        // MM-DD-YYYY
    int date_val;         // Computed YYYYMMDD integer for fast comparison
    int order;            // Sequence tracking for same date/category tie-breaking
    PriorityTier priority;
} Email;

// Structure for the MaxHeap
typedef struct {
    Email *emails;
    int size;
    int capacity;
} MaxHeap;

// ---------------------------------------------------------
// Maps category string to enum priority level
// ---------------------------------------------------------
PriorityTier getPriority(const char *category) {
    if (strcmp(category, "Boss") == 0) {
        return PRIORITY_BOSS;
    }
    if (strcmp(category, "OtherPerson") == 0) {
        return PRIORITY_OTHER;
    }
    // Subordinate, Peer, and ImportantPerson all share the middle tier
    return PRIORITY_MID;
}

// ---------------------------------------------------------
// Converts MM-DD-YYYY into a comparable YYYYMMDD integer
// ---------------------------------------------------------
int parseDateValue(const char *date) {
    int month = 0, day = 0, year = 0;
    sscanf(date, "%d-%d-%d", &month, &day, &year);
    return (year * 10000) + (month * 100) + day;
}

// ---------------------------------------------------------
// Determines if email a has higher priority than email b
// Executed in O(1) time without string or sscanf overhead
// ---------------------------------------------------------
int higherPriority(const Email *a, const Email *b) {
    // 1. Sender Category
    if (a->priority != b->priority) {
        return a->priority > b->priority;
    }

    // 2. Date (Newest date = larger integer value = higher priority)
    if (a->date_val != b->date_val) {
        return a->date_val > b->date_val;
    }

    // 3. Arrival Order (Later arrival considered newer if date is identical)
    return a->order > b->order;
}

// ---------------------------------------------------------
// Swap function inline helper
// ---------------------------------------------------------
static inline void swapEmails(Email *a, Email *b) {
    Email temp = *a;
    *a = *b;
    *b = temp;
}

// ---------------------------------------------------------
// Initialize MaxHeap dynamically
// ---------------------------------------------------------
void initializeHeap(MaxHeap *heap) {
    heap->emails = (Email *)malloc(INITIAL_CAPACITY * sizeof(Email));
    if (heap->emails == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        exit(1);
    }
    heap->size = 0;
    heap->capacity = INITIAL_CAPACITY;
}

// ---------------------------------------------------------
// Dynamic heap growth
// ---------------------------------------------------------
void resizeHeap(MaxHeap *heap) {
    heap->capacity *= 2;
    Email *temp = (Email *)realloc(heap->emails, heap->capacity * sizeof(Email));

    if (temp == NULL) {
        fprintf(stderr, "Memory allocation error.\n");
        free(heap->emails);
        exit(1);
    }
    heap->emails = temp;
}

// ---------------------------------------------------------
// Upward heap adjustment
// ---------------------------------------------------------
void heapifyUp(MaxHeap *heap, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;

        if (higherPriority(&heap->emails[index], &heap->emails[parent])) {
            swapEmails(&heap->emails[index], &heap->emails[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

// ---------------------------------------------------------
// Downward heap adjustment
// ---------------------------------------------------------
void heapifyDown(MaxHeap *heap, int index) {
    while (1) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        if (left < heap->size && higherPriority(&heap->emails[left], &heap->emails[largest])) {
            largest = left;
        }

        if (right < heap->size && higherPriority(&heap->emails[right], &heap->emails[largest])) {
            largest = right;
        }

        if (largest == index) {
            break;
        }

        swapEmails(&heap->emails[index], &heap->emails[largest]);
        index = largest;
    }
}

// ---------------------------------------------------------
// Enqueue element into MaxHeap
// ---------------------------------------------------------
void insertEmail(MaxHeap *heap, Email email) {
    if (heap->size == heap->capacity) {
        resizeHeap(heap);
    }

    heap->emails[heap->size] = email;
    heapifyUp(heap, heap->size);
    heap->size++;
}

// ---------------------------------------------------------
// Inspect top element without removing
// ---------------------------------------------------------
Email *peek(MaxHeap *heap) {
    if (heap->size == 0) return NULL;
    return &heap->emails[0];
}

// ---------------------------------------------------------
// Dequeue top priority element
// ---------------------------------------------------------
void removeMax(MaxHeap *heap) {
    if (heap->size == 0) return;

    heap->emails[0] = heap->emails[heap->size - 1];
    heap->size--;

    if (heap->size > 0) {
        heapifyDown(heap, 0);
    }
}

// ---------------------------------------------------------
// String trimmer for robust command/data parsing
// ---------------------------------------------------------
void trim(char *str) {
    char *start = str;

    while (*start == ' ' || *start == '\t') {
        start++;
    }

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    int length = strlen(str);
    while (length > 0 &&
           (str[length - 1] == ' '  ||
            str[length - 1] == '\t' ||
            str[length - 1] == '\n' ||
            str[length - 1] == '\r')) {
        str[length - 1] = '\0';
        length--;
    }
}

// ---------------------------------------------------------
// Main execution loop
// ---------------------------------------------------------
int main(void) {
    MaxHeap heap;
    initializeHeap(&heap);

    char line[512];
    int orderCounter = 0;

    while (fgets(line, sizeof(line), stdin) != NULL) {
        trim(line);

        if (strlen(line) == 0) continue;

        if (strncmp(line, "EMAIL ", 6) == 0) {
            char *data = line + 6;

            char *category = strtok(data, ",");
            char *subject  = strtok(NULL, ",");
            char *date     = strtok(NULL, ",");

            if (category && subject && date) {
                trim(category);
                trim(subject);
                trim(date);

                Email email;
                strncpy(email.category, category, sizeof(email.category) - 1);
                strncpy(email.subject, subject, sizeof(email.subject) - 1);
                strncpy(email.date, date, sizeof(email.date) - 1);

                // Add null terminators safely
                email.category[sizeof(email.category) - 1] = '\0';
                email.subject[sizeof(email.subject) - 1]   = '\0';
                email.date[sizeof(email.date) - 1]         = '\0';

                // Precompute variables for immediate O(1) comparison
                email.priority = getPriority(email.category);
                email.date_val = parseDateValue(email.date);
                email.order    = orderCounter++;

                insertEmail(&heap, email);
            }
        } 
        else if (strcmp(line, "NEXT") == 0) {
            Email *email = peek(&heap);
            if (email != NULL) {
                printf("Sender: %s\n", email->category);
                printf("Subject: %s\n", email->subject);
                printf("Date: %s\n", email->date);
            }
        } 
        else if (strcmp(line, "READ") == 0) {
            removeMax(&heap);
        } 
        else if (strcmp(line, "COUNT") == 0) {
            printf("%d\n", heap.size);
        }
    }

    free(heap.emails);
    return 0;
}
