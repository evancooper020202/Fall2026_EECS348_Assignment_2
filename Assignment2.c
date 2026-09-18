 /*******************************************************************************
 #* Program Name: EECS 348 Assignment - Email Priority Queue Manager
 #*
 * Description:
 *      A C program that manages and prioritizes incoming emails for a CEO using
 *      a list-based MaxHeap priority queue. Emails are prioritized first by sender 
 *      category (Boss > Subordinate > Peer > ImportantPerson > OtherPerson), 
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
 *      Formatted text displayed to the terminal based on commands processed 
 *      (e.g., email details, current count of unread messages).
 *
 * Collaborators: None
 * Other Sources: ChatGPT / AI Assistance
 *
 * Author: [Evan Cooper]
 * Creation Date: September 17, 2026
 * Revision Date: September 17, 2026
 * Revisions:
 *      - Refactored heap comparison logic and added robust string trimming routines.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10

// Structure for an email
typedef struct {
    char category[30];
    char subject[200];
    char date[11];       // MM-DD-YYYY
    int priority;
    int order;           // Used when category and date are the same
} Email;

// Structure for the MaxHeap
typedef struct {
    Email *emails;
    int size;
    int capacity;
} MaxHeap;


// ---------------------------------------------------------
// Return the priority value for each sender category
// Higher number = higher priority
// ---------------------------------------------------------
int getPriority(char category[]) {
    if (strcmp(category, "Boss") == 0)
        return 5;
    else if (strcmp(category, "Subordinate") == 0)
        return 4;
    else if (strcmp(category, "Peer") == 0)
        return 3;
    else if (strcmp(category, "ImportantPerson") == 0)
        return 2;
    else
        return 1;   // OtherPerson
}


// ---------------------------------------------------------
// Convert MM-DD-YYYY into a number that can be compared
// ---------------------------------------------------------
int dateValue(char date[]) {
    int month, day, year;

    sscanf(date, "%d-%d-%d", &month, &day, &year);

    return year * 10000 + month * 100 + day;
}


// ---------------------------------------------------------
// Determine whether email a has higher priority than email b
// ---------------------------------------------------------
int higherPriority(Email a, Email b) {

    // First compare sender category
    if (a.priority > b.priority)
        return 1;

    if (a.priority < b.priority)
        return 0;

    // Same category: compare dates
    int dateA = dateValue(a.date);
    int dateB = dateValue(b.date);

    if (dateA > dateB)
        return 1;

    if (dateA < dateB)
        return 0;

    // Same category and date:
    // later email entered is considered newer
    if (a.order > b.order)
        return 1;

    return 0;
}


// ---------------------------------------------------------
// Swap two emails
// ---------------------------------------------------------
void swapEmails(Email *a, Email *b) {
    Email temp = *a;
    *a = *b;
    *b = temp;
}


// ---------------------------------------------------------
// Initialize the MaxHeap
// ---------------------------------------------------------
void initializeHeap(MaxHeap *heap) {
    heap->emails = malloc(INITIAL_CAPACITY * sizeof(Email));

    if (heap->emails == NULL) {
        printf("Memory allocation error.\n");
        exit(1);
    }

    heap->size = 0;
    heap->capacity = INITIAL_CAPACITY;
}


// ---------------------------------------------------------
// Resize the heap when it becomes full
// ---------------------------------------------------------
void resizeHeap(MaxHeap *heap) {
    heap->capacity *= 2;

    Email *temp = realloc(
        heap->emails,
        heap->capacity * sizeof(Email)
    );

    if (temp == NULL) {
        printf("Memory allocation error.\n");
        free(heap->emails);
        exit(1);
    }

    heap->emails = temp;
}


// ---------------------------------------------------------
// Move an item upward in the MaxHeap
// ---------------------------------------------------------
void heapifyUp(MaxHeap *heap, int index) {

    while (index > 0) {

        int parent = (index - 1) / 2;

        if (higherPriority(
                heap->emails[index],
                heap->emails[parent])) {

            swapEmails(
                &heap->emails[index],
                &heap->emails[parent]
            );

            index = parent;
        }
        else {
            break;
        }
    }
}


// ---------------------------------------------------------
// Move an item downward in the MaxHeap
// ---------------------------------------------------------
void heapifyDown(MaxHeap *heap, int index) {

    while (1) {

        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;

        // Check left child
        if (left < heap->size &&
            higherPriority(
                heap->emails[left],
                heap->emails[largest])) {

            largest = left;
        }

        // Check right child
        if (right < heap->size &&
            higherPriority(
                heap->emails[right],
                heap->emails[largest])) {

            largest = right;
        }

        // No child has higher priority
        if (largest == index)
            break;

        swapEmails(
            &heap->emails[index],
            &heap->emails[largest]
        );

        index = largest;
    }
}


// ---------------------------------------------------------
// Insert an email into the MaxHeap
// ---------------------------------------------------------
void insertEmail(MaxHeap *heap, Email email) {

    if (heap->size == heap->capacity) {
        resizeHeap(heap);
    }

    // Put the new email at the end
    heap->emails[heap->size] = email;

    // Restore MaxHeap property
    heapifyUp(heap, heap->size);

    heap->size++;
}


// ---------------------------------------------------------
// Return the highest-priority email without removing it
// ---------------------------------------------------------
Email *peek(MaxHeap *heap) {

    if (heap->size == 0)
        return NULL;

    return &heap->emails[0];
}


// ---------------------------------------------------------
// Remove the highest-priority email
// ---------------------------------------------------------
void removeMax(MaxHeap *heap) {

    if (heap->size == 0)
        return;

    // Move last item to the root
    heap->emails[0] = heap->emails[heap->size - 1];

    heap->size--;

    // Restore MaxHeap property
    if (heap->size > 0) {
        heapifyDown(heap, 0);
    }
}


// ---------------------------------------------------------
// Remove leading/trailing whitespace
// ---------------------------------------------------------
void trim(char *str) {

    // Remove leading spaces
    char *start = str;

    while (*start == ' ' || *start == '\t')
        start++;

    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }

    // Remove trailing spaces
    int length = strlen(str);

    while (length > 0 &&
           (str[length - 1] == ' ' ||
            str[length - 1] == '\t' ||
            str[length - 1] == '\n' ||
            str[length - 1] == '\r')) {

        str[length - 1] = '\0';
        length--;
    }
}


// ---------------------------------------------------------
// Main program
// ---------------------------------------------------------
int main(void) {

    MaxHeap heap;
    initializeHeap(&heap);

    char line[500];
    int orderCounter = 0;

    // Read commands until end of file
    while (fgets(line, sizeof(line), stdin) != NULL) {

        trim(line);

        // Ignore empty lines
        if (strlen(line) == 0)
            continue;


        // -------------------------------------------------
        // EMAIL command
        // Format:
        // EMAIL category,subject,date
        // -------------------------------------------------
        if (strncmp(line, "EMAIL ", 6) == 0) {

            char *data = line + 6;

            char *category = strtok(data, ",");
            char *subject = strtok(NULL, ",");
            char *date = strtok(NULL, ",");

            if (category != NULL &&
                subject != NULL &&
                date != NULL) {

                trim(category);
                trim(subject);
                trim(date);

                Email email;

                strcpy(email.category, category);
                strcpy(email.subject, subject);
                strcpy(email.date, date);

                email.priority = getPriority(email.category);

                // Used to determine which email is newer
                email.order = orderCounter++;

                insertEmail(&heap, email);
            }
        }


        // -------------------------------------------------
        // NEXT command
        // Display highest-priority email
        // without removing it
        // -------------------------------------------------
        else if (strcmp(line, "NEXT") == 0) {

            Email *email = peek(&heap);

            if (email != NULL) {

                printf("Sender: %s\n", email->category);
                printf("Subject: %s\n", email->subject);
                printf("Date: %s\n", email->date);
            }
        }


        // -------------------------------------------------
        // READ command
        // Remove highest-priority email
        // -------------------------------------------------
        else if (strcmp(line, "READ") == 0) {

            removeMax(&heap);
        }


        // -------------------------------------------------
        // COUNT command
        // Display number of unread emails
        // -------------------------------------------------
        else if (strcmp(line, "COUNT") == 0) {

            printf("%d\n", heap.size);
        }
    }


    // Free dynamically allocated memory
    free(heap.emails);

    return 0;
}