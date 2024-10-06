#include <threading.h>
//#include <stdio.h>

void t_init()
{
	//save current context and then change the state and current_context_idx
	getcontext(&contexts[0].context);
	contexts[0].state = VALID;
    current_context_idx = 0;
}

int32_t t_create(fptr foo, int32_t arg1, int32_t arg2)
{
	//uses makecontext to create a new context that can be switched to
	volatile int i = 0;
	while (contexts[i].state != INVALID && i < NUM_CTX)
	{
		i++;
	}
	
	//check if unable to find INVALID context
	if (contexts[i].state != INVALID)
	{
		return 1;
	}
	
	//try to save the context, if it fails then we exit
	if (getcontext(&contexts[i].context) != 0)
	{
		return 1;
	}
	
	//allocate stack and change new contexts stack pointer
	char* stack = malloc(STK_SZ);
	contexts[i].context.uc_stack.ss_sp = stack;
    contexts[i].context.uc_stack.ss_size = STK_SZ;
    //change the context link so the context can be switched back
	contexts[i].context.uc_link = &(contexts[current_context_idx].context);
	//make context and adjust state to valid
	makecontext(&contexts[i].context, (void (*)(void))foo, 2, arg1, arg2);
	contexts[i].state = VALID;
	return 0;
}

//swap context
int32_t t_yield()
{
	//save current context
	getcontext(&contexts[current_context_idx].context);
	
	uint8_t i = 0;
	//find VALID context if any
	while (i < NUM_CTX && (contexts[i].state != VALID || i == current_context_idx))
	{
		i++;
	}
	//only main context left
	if (i == NUM_CTX)
	{
		return 0;
	}

	//printf("i: %d\n", i);
	//printf("cci: %d\n", current_context_idx);
	
	//swapping logic
	int oldContextID = current_context_idx;
	current_context_idx = i;
	swapcontext(&contexts[oldContextID].context, &contexts[i].context);
	
	//count valid contexts
	int validCount = 0;
	for (int j = 0; j < NUM_CTX; j++)
	{
		if (contexts[j].state == VALID)
		{
			validCount++;
		}
	}
	return validCount;
}

void t_finish()
{
	//deletes and resets everything about the current new context,
    //need to update contexts array and current context id I think
	char* stack = contexts[current_context_idx].context.uc_stack.ss_sp;
	free(stack);
	contexts[current_context_idx].state = DONE;
	contexts[current_context_idx].context.uc_stack.ss_sp = NULL;
	contexts[current_context_idx].context.uc_stack.ss_size = 0;
	//memset(&contexts[current_context_idx].context, 0, sizeof(contexts[current_context_idx].context));
}
