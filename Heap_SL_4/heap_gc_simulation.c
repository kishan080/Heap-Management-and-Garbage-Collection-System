// Name: Kishan Verma
// Roll no: BT20CSE120.    SL4

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HEAP_SIZE 4181 // max size that can be allocated. fib(20)=4181
#define N 20           // free list size min 1 to 20
//==============================================================================//
typedef struct Node_allocate_tag
{
    char tag_name[50];
    int start_point;
    int end_point;
    int size;
    int size_demand;
    int pos; // postion in freelist
    struct Node_allocate_tag *next;

    // adjacent list/matrix
    // for simplicity at most am keeping it as 3 same as in diagram in assignment
    //  instead we can have Linked head which have all other refernces..
    struct Node_allocate_tag *adj1;
    struct Node_allocate_tag *adj2;
    struct Node_allocate_tag *adj3;

    // for reference counting
    int count;
    int mark;

} alloc;

alloc *head_ptr = NULL; // head pointer to allocted list

typedef struct Node_Free_list_tag
{
    int start_point;
    int size;
    struct Node_Free_list_tag *next;
} FreeNode;

typedef struct FreeList_tag
{
    int heap_size; // fibonacii size allowed;
    FreeNode *head;
} FreeList_tag;

//===================================================================================//
FreeList_tag Free_list[N];

int fib[N]; // preprocessing fibonacci number for faster access

void intialize_fib_Free_list(FreeNode *freeptr)
{
    fib[0] = 0;
    fib[1] = 1;
    for (int i = 2; i < N; i++)
    {
        Free_list[i].heap_size = fib[i] = fib[i - 1] + fib[i - 2];
        Free_list[i].head = NULL;
    }
    Free_list[N - 1].head = freeptr;
    // intialize freelist size also with approiate size at same time
}
//--------------------------------------//

int FindPos(int size) // find position in Free_list which fits given size
{
    // using binary search we can find position very efficiently since fibonacci is
    // in sorted order
    int l = 2;
    int h = N - 1;
    int ans = 2;

    while (l <= h)
    {
        int mid = (l + h) / 2;

        if (fib[mid] >= size)
        {
            ans = mid;
            h = mid - 1;
        }
        else // fib[mid]<size
            l = mid + 1;
    }
    return ans;
}
//---------------------------------------------------------------------------------------------------//
void insertAtFreeList(int i, int start_pnt) // insert at position i refrence to x
{

    FreeNode *temp = (FreeNode *)malloc(sizeof(FreeNode));
    temp->size = fib[i];
    temp->start_point = start_pnt;

    if (Free_list[i].head == NULL)
    {
        Free_list[i].head = temp;
        temp->next = NULL;
    }
    else
    {
        FreeNode *free_prev = NULL;
        FreeNode *freetemp = Free_list[i].head;

        while (freetemp != NULL && freetemp->start_point < temp->start_point)
        {
            free_prev = freetemp;
            freetemp = freetemp->next;
        }

        if (free_prev == NULL)
        {
            Free_list[i].head = temp;
            temp->next = freetemp;
        }
        else
        {
            free_prev->next = temp;
            temp->next = freetemp;
        }
    }
}

//--------------------------------------------------------------------------------------------------//

//--------------------------------------------------------------------------------------------------//
alloc *insertInAlloc(char *t_name, int pos, int size)
{
    FreeNode *ptr = Free_list[pos].head;

    alloc *new_node = (alloc *)malloc(sizeof(alloc));
    new_node->next = NULL;
    strcpy(new_node->tag_name, t_name);
    new_node->size = ptr->size;
    new_node->size_demand = size;
    new_node->start_point = ptr->start_point;
    new_node->end_point = ptr->start_point + ptr->size - 1;
    new_node->pos = pos;

    //-------------------//
    new_node->adj1 = NULL;
    new_node->adj2 = NULL;
    new_node->adj3 = NULL;
    //-------------------//
    new_node->count = 0;
    new_node->mark = 0;
    //-------------------//

    alloc *prev = NULL;

    if (head_ptr == NULL)
    {
        head_ptr = new_node;
    }
    else
    {
        alloc *ahead = head_ptr;

        while (ahead != NULL && ahead->start_point < new_node->start_point)
        {
            prev = ahead;
            ahead = ahead->next;
        }
        if (prev == NULL)
        {
            head_ptr = new_node;
            new_node->next = ahead;
        }
        else
        {
            prev->next = new_node;
            new_node->next = ahead;
        }
    }
    Free_list[pos].head = ptr->next;
    free(ptr);

    return new_node;
}
//--------------------------------------------------------------------------------------------------//

alloc *Malloc(char *t_name /* tag_name*/, int size /*size to be allocate*/)
{

    int pos = FindPos(size); // find position in Free_list which fits given size

    if (Free_list[pos].head == NULL)
    {
        int next_pos = pos + 1; // find next position in freelist
                                // which is not empty that is not NULL.
        int found = 0;
        while (next_pos < N && !found)
        {
            if (Free_list[next_pos].head != NULL)
                found = 1;
            else
                next_pos++;
        }

        if (!found) // if no position is empty means is heap is FUll for this allocation
        {
            printf("___Heap Overflow!!!___\n");
        }
        else
        {
            // if  we found next position break in smaller chunks i.e fib[i-1],fib[i-2]
            while (Free_list[pos].head == NULL)
            {
                FreeNode *ptr = Free_list[next_pos].head;
                /*--at--*/
                insertAtFreeList(next_pos - 1, ptr->start_point + fib[next_pos - 2]);
                insertAtFreeList(next_pos - 2, ptr->start_point);

                Free_list[next_pos].head = ptr->next;
                free(ptr);
                next_pos -= 2; // now break smaller further if didnt get desire size.
            }
        }
    }

    if (Free_list[pos].head != NULL)
    {
        return insertInAlloc(t_name, pos, size);
    }
    else
        return NULL;
}

///----------------------------------------------------------------------------------//
void merge_free()
{
    for (int i = 2; i < N - 2; i++)
    {
        if (Free_list[i].head != NULL)
        {
            // look for its buddy..
            if (Free_list[i + 1].head != NULL)
            {
                FreeNode *temp = Free_list[i].head;
                FreeNode *prev_temp = NULL;

                while (temp != NULL && Free_list[i + 1].head != NULL)
                {
                    FreeNode *buddy = Free_list[i + 1].head;
                    FreeNode *prev_buddy = NULL;

                    while (buddy != NULL)
                    {
                        if (temp->start_point + temp->size == buddy->start_point)
                        {
                            // combine and insert to next list..
                            insertAtFreeList(i + 2, temp->start_point);

                            // delete buddy and temp
                            if (prev_temp == NULL)
                                Free_list[i].head = temp->next;
                            else
                                prev_temp->next = temp->next;

                            // delete buddy
                            if (prev_buddy == NULL)
                                Free_list[i + 1].head = buddy->next;
                            else
                                prev_buddy->next = buddy->next;

                            free(temp);
                            free(buddy);
                        }
                        else
                        {
                            prev_buddy = buddy;
                            buddy = buddy->next;
                        }
                    }
                    prev_temp = temp;
                    temp = temp->next;
                }
            }
        }
    }
}
///----------------------------------------------------------------------------------//

void Free(char *t_name)
{
    alloc *tptr = head_ptr;
    alloc *prevptr = NULL;

    if (tptr == NULL)
    {
        printf("!!!_________heap Underflow__________!!!\n");
        return;
    }
    else
    {
        while (tptr != NULL && strcmp(tptr->tag_name, t_name) != 0)
        {
            prevptr = tptr;
            tptr = tptr->next;
        }
        if (tptr == NULL)
        {
            printf("%s :-___Not found in list ___!!!\n", t_name);
            return;
        }
        else
        {

            if (prevptr != NULL)
            {
                printf("!!!!___Memory  %s is freed___!!!!\n", t_name);
                prevptr->next = tptr->next;
            }
            else
            {
                printf("!!!____Memory  %s is freed___!!!!\n", t_name);
                head_ptr = tptr->next;
            }
        }
    }

    insertAtFreeList(tptr->pos, tptr->start_point);
    free(tptr);

    merge_free();
}

///----------------------------------------------------------------------------------//

///----------------------------------------------------------------------------------//
void Print_alloc_list()
{
    alloc *lptr = head_ptr;
    if (lptr != NULL)
    {
        while (lptr != NULL)
        {
            printf("Tag name:%s\n", lptr->tag_name);
            printf("start_point:%d ", lptr->start_point);
            printf("\tend_point:%d\n", lptr->end_point);
            printf("Size allocated :%d\n", lptr->size);
            printf("Size demanded :%d\n", lptr->size_demand);
            lptr = lptr->next;
            printf("\n");
        }
    }
    else
    {
        printf("!!!!_____Empty allocation list______!!!!\n");
    }
}
///----------------------------------------------------------------------------------//
///----------------------------------------------------------------------------------//
void Print_Free_list()
{
    FreeNode *temp;
    int i = 2;
    while (i < N)
    {
        if (Free_list[i].head != NULL)
        {
            temp = Free_list[i].head;
            printf("-------------------------------------------\n");
            printf("SIZE %d IS:\n", Free_list[i].heap_size);
            while (temp != NULL)
            {
                printf("start=%d \t\tend=%d	SIZE=%d\n", temp->start_point, temp->start_point + temp->size - 1, temp->size);
                temp = temp->next;
            }
            printf("\n");
        }

        i++;
    }
    printf("-------------------------------------------\n");
}

///----------------------------------------------------------------------------------//
int validateName(char *tag_name)
{
    alloc *temp = head_ptr;
    int flag = 1;
    while (temp != NULL && flag)
    {
        if (strcmp(tag_name, temp->tag_name) == 0)
            flag = 0;
        temp = temp->next;
    }

    if (flag == 0)
        printf(" This tag_name already exits!!\n");
    return flag;
}

int validateSize(int size)
{
    return size <= HEAP_SIZE && size >= 1;
}

///----------------------------------------------------------------------------------//

typedef struct variable_tag
{
    alloc *root;
} variable_ptr;

void genrate_heap_simulation(variable_ptr **root1, variable_ptr **root2)
{
    variable_ptr *temp = *root1;
    temp->root = Malloc("T5", 2); // name, size return address of memory
    temp->root->count = 2;

    (*root2)->root = temp->root->adj1 = Malloc("T1", 2);
    temp->root->adj1->count = 3;

    temp->root->adj1->adj1 = Malloc("T2", 2);
    temp->root->adj1->adj1->count = 1;
    temp->root->adj1->adj2 = Malloc("T9", 2);
    temp->root->adj1->adj2->count = 2;
    temp->root->adj1->adj3 = Malloc("T10", 2);
    temp->root->adj1->adj3->count = 2;

    alloc *temp2 = Malloc("T7", 2);
    temp2->count = 0;
    temp2->adj1 = temp->root->adj1;
    temp2->adj2 = Malloc("T8", 2);
    temp2->adj2->count = 2;
    temp2->adj2->adj1 = temp->root->adj1->adj2;

    alloc *temp3 = Malloc("T3", 2);
    temp3->adj1 = temp2->adj2;
    temp3->adj2 = temp->root->adj1->adj3;
}

///----------------------------------------------------------------------------------//

void Print_varible(alloc *root)
{
    printf("%s ", root->tag_name);

    if (root->adj1 == NULL && root->adj2 == NULL && root->adj3 == NULL)
        return;

    printf("--> {");
    if (root->adj1 != NULL)
        Print_varible(root->adj1);
    if (root->adj2 != NULL)
        Print_varible(root->adj2);
    if (root->adj3 != NULL)
        Print_varible(root->adj3);
    printf(" }");
}
///----------------------------------------------------------------------------------//
void Reference_counting_GC()
{
    alloc *temp, *prev;

    int isThere = 1;
    while (isThere) // unitl there there is node count =0 keep deleting them
    {
        isThere = 0;
        temp = head_ptr;
        prev = NULL;

        while (temp != NULL)
        {
            int delete_flag = 0;

            if (temp->count == 0)
            {
                isThere = 1;

                delete_flag = 1;

                if (temp->adj1 != NULL)
                {
                    (temp->adj1->count) -= 1;
                }
                if (temp->adj2 != NULL)
                {
                    (temp->adj2->count) -= 1;
                }
                if (temp->adj3 != NULL)
                {
                    (temp->adj3->count) -= 1;
                }

                // delete node or free up the data;
                alloc *temp2;
                if (prev == NULL)
                {
                    temp2 = head_ptr;
                    temp = head_ptr = head_ptr->next;
                }
                else
                {
                    temp2 = temp;
                    prev->next = temp->next;
                    temp = temp->next;
                }

                // insert into free list.
                insertAtFreeList(temp2->pos, temp2->start_point);
                free(temp2);
            }

            if (delete_flag == 0)
            {
                prev = temp;
                temp = temp->next;
            }
        }
    }

    printf("\n--------Refrence counting ok..---------\n");
}

///----------------------------------------------------------------------------------//
void mark_phase(variable_ptr *rootn)
{
    alloc *temp = rootn->root;
    alloc *tail = NULL, *middle = NULL;

    while (temp != NULL)
    {
        if (temp->mark == 0)
        {
            temp->mark = 1;
        }
        if (temp->adj1 != NULL && temp->adj1->mark == 0)
        {
            tail = middle;
            middle = temp;
            temp = temp->adj1;
        }
        else if (temp->adj2 != NULL && temp->adj2->mark == 0)
        {
            tail = middle;
            middle = temp;
            temp = temp->adj2;
        }
        else if (temp->adj3 != NULL && temp->adj3->mark == 0)
        {
            tail = middle;
            middle = temp;
            temp = temp->adj3;
        }
        else
        {
            temp = middle;
            middle = tail;
            tail = NULL;
        }
    }
}

void sweep_phase()
{
    alloc *temp, *prev;

    temp = head_ptr;
    prev = NULL;

    while (temp != NULL)
    {
        int delete_flag = 0;

        if (temp->mark == 0)
        {

            delete_flag = 1;

            // delete node or free up the data;
            alloc *temp2;
            if (prev == NULL)
            {
                temp2 = head_ptr;
                temp = head_ptr = head_ptr->next;
            }
            else
            {
                temp2 = temp;
                prev->next = temp->next;
                temp = temp->next;
            }

            // insert into free list.
            insertAtFreeList(temp2->pos, temp2->start_point);
            free(temp2);
        }
        else
        {
            temp->mark = 0; // undo mark flag;
        }

        if (delete_flag == 0)
        {
            prev = temp;
            temp = temp->next;
        }
    }

    printf("\n--------marks sweep done---------\n");
}

void marks_sweep_GC(variable_ptr *root1, variable_ptr *root2)
{
    // mark phase
    mark_phase(root1);
    mark_phase(root2);
    printf("\n---mark phase done---\n");
    // sweep phase
    sweep_phase();
}
///----------------------------------------------------------------------------------//

int main()
{

    FreeNode *ptr = (FreeNode *)malloc(sizeof(FreeNode));
    ptr->start_point = 0;
    ptr->next = NULL;
    ptr->size = HEAP_SIZE;

    intialize_fib_Free_list(ptr); // genrate fibonacci series and intialize Free list;

    variable_ptr *root1 = (variable_ptr *)malloc(sizeof(variable_ptr));
    variable_ptr *root2 = (variable_ptr *)malloc(sizeof(variable_ptr));

    root1->root = NULL;
    root2->root = NULL;

    genrate_heap_simulation(&root1, &root2);

    printf("\n------HEAP ALLOCATED DATA-----\n");
    Print_alloc_list();

    printf("\nroot1-->");
    Print_varible(root1->root);
    printf("\nroot2-->");
    Print_varible(root2->root);

    printf("\n---------FREE LIST----------\n");
    Print_Free_list();

    printf("\n==================================================================\n");
    printf("                           MENU                              \n");
    printf("==================================================================\n");
    printf("1.Simulate reference counting\n");
    printf("2.Simulate mark and sweep\n");

    int choice;
    printf("Enter choice: ");
    scanf("%d", &choice);

    switch (choice)
    {
    case 1:
    {
        Reference_counting_GC();
    }
    break;
    case 2:
    {
        marks_sweep_GC(root1, root2);
    }
    break;

    default:
        printf("\nEntered Wrong choice!! Try again..\n");
        break;
    }

    printf("\n------HEAP ALLOCATED DATA after GC-----\n");

    Print_alloc_list();
    printf("\n---------FREE LIST after GC----------\n");
    Print_Free_list();

    /* // CPL 3 heap manipilation...
    int flag = 1;

    while (flag)
    {
        printf("==================================================================\n");
        printf("                           MENU                              \n");
        printf("==================================================================\n");
        printf("1.malloc\n");
        printf("2.free\n");
        printf("3.print_allocated_data\n");
        printf("4.print_free_list_ data\n");
        printf("5.exit\n");

        int choice, size;
        char tag_name[50];
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:

            // validate tag_name .. no two allocatation  should have same name:
            do
            {
                printf("Enter tag_name: ");
                scanf("%s", tag_name);
                fflush(stdin);
            } while (validateName(tag_name) == 0);

            do
            {
                printf("Enter size to be allocated(size <=4181 and size>=1 ): ");
                scanf("%d", &size);
            } while (validateSize(size) == 0);

            alloc *temp = Malloc(tag_name, size);

            break;
        case 2:
            printf("Enter tag_name: ");
            scanf("%s", tag_name);
            Free(tag_name);
            break;
        case 3:
            printf("-----------------------------------------------------\n");
            printf("Allocated list data: \n");
            Print_alloc_list();
            printf("-----------------------------------------------------\n");
            break;
        case 4:
            printf("-----------------------------------------------------\n");
            printf("free list data: \n");
            Print_Free_list();
            printf("-----------------------------------------------------\n");
            break;
        case 5:
            flag = 0;
            break;

        default:
            printf("Enter valid choice:\n");
            break;
        }
    }
    */

    return 0;
}