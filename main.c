#include <stdio.h>

#define HEAP_SIZE 65536
#define MAX_BLOCKS 1024

char heap[HEAP_SIZE];

int block_offset[MAX_BLOCKS];
int block_size[MAX_BLOCKS];
int block_free[MAX_BLOCKS];
int block_count = 0;

void heap_init()
{
	block_offset[0] = 0;
	block_size[0] = HEAP_SIZE;
	block_free[0] = 1;
	block_count = 1;
}

// 인접한 빈 블록들을 합쳐서 외부 단편화를 줄인다.
void coalesce()
{
	int merged = 1;

	while (merged == 1)
	{
		merged = 0;

		for (int i = 0; i < block_count - 1; )
		{
			int end_of_current = block_offset[i] + block_size[i];

			if (block_free[i] == 1 && block_free[i + 1] == 1 && end_of_current == block_offset[i + 1])
			{
				block_size[i] += block_size[i + 1];

				// i+1 블록을 삭제하고 뒤 블록들을 한 칸씩 앞으로 당긴다.
				for (int j = i + 1; j < block_count - 1; j++)
				{
					block_offset[j] = block_offset[j + 1];
					block_size[j] = block_size[j + 1];
					block_free[j] = block_free[j + 1];
				}
				block_count -= 1;
				merged = 1;
			}
			else
			{
				i += 1;
			}
		}
	}
}

// first-fit: size 이상인 첫 빈 블록을 찾아, 필요한 만큼만 잘라서 내어준다 (분할)
char *my_malloc(int size)
{
	if (size <= 0) return NULL;
	if (block_count == 0) heap_init();

	for (int i = 0; i < block_count; i++)
	{
		if (block_free[i] == 1 && block_size[i] >= size)
		{
			// 남는 공간이 있고 블록 배열에 여유가 있으면 뒤쪽을 새 빈 블록으로 분할한다.
			if (block_size[i] > size && block_count < MAX_BLOCKS)
			{
				// i+1 자리를 비우기 위해 뒤 블록들을 한 칸씩 뒤로 민다.
				for (int j = block_count; j > i + 1; j--)
				{
					block_offset[j] = block_offset[j - 1];
					block_size[j] = block_size[j - 1];
					block_free[j] = block_free[j - 1];
				}

				block_offset[i + 1] = block_offset[i] + size;
				block_size[i + 1] = block_size[i] - size;
				block_free[i + 1] = 1;

				block_size[i] = size;
				block_count += 1;
			}

			block_free[i] = 0;
			return heap + block_offset[i];
		}
	}
	return NULL;
}

void my_free(char *ptr)
{
	if (ptr == NULL) return;

	int offset = ptr - heap;

	int i = 0;
	while (i < block_count)
	{
		if (block_offset[i] == offset)
		{
			block_free[i] = 1;
			i = block_count;
		}
		else
		{
			i += 1;
		}
	}

	coalesce();
}

void heap_dump()
{
	printf("==== 힙 상태 ====\n");

	for (int i = 0; i < block_count; i++)
	{
		if (block_free[i] == 1)
		{
			printf("블록 %d | 오프셋 +%d | 크기 %d바이트 | 비어있음\n",
			       i, block_offset[i], block_size[i]);
		}
		else
		{
			printf("블록 %d | 오프셋 +%d | 크기 %d바이트 | 사용중 \n",
			       i, block_offset[i], block_size[i]);
		}
	}
	printf("=================\n\n");
}

void heap_stats()
{
	int free_count = 0;
	int used_count = 0;
	int largest_free = 0;

	for (int i = 0; i < block_count; i++)
	{
		if (block_free[i] == 1)
		{
			free_count += 1;
			if (block_size[i] > largest_free)
			{
				largest_free = block_size[i];
			}
		}
		else
		{
			used_count += 1;
		}
	}
	printf("[통계] 빈 블록 %d개 | 사용 블록 %d개 | 가장 큰 빈 블록 %d바이트\n\n",
	       free_count, used_count, largest_free);
}

int main()
{
	char *ptrs[MAX_BLOCKS] = {0};

	printf("명령어: m <slot> <size> (할당) / f <slot> (해제) / d (덤프) / s (통계) / r (힙 초기화) / c (화면 지우기) / q (종료)\n\n");

	char cmd;

	while (1)
	{
		printf("> ");
		if (scanf(" %c", &cmd) != 1) break;

		if (cmd == 'm') // 메모리 할당
		{
			int size, slot;
			scanf("%d %d", &slot, &size);
			ptrs[slot] = my_malloc(size);
			printf("slot %d -> %s\n\n", slot, ptrs[slot] ? "할당 성공" : "할당 실패");

		}

		else if (cmd == 'f') // 메모리 해제
		{
			int slot;
			scanf("%d", &slot);
			my_free(ptrs[slot]);
			ptrs[slot] = NULL;
			printf("slot %d 해제\n\n", slot);

		}

		else if (cmd == 'd') // 힙 상태 출력
		{
			heap_dump();

		}

		else if (cmd == 's') // 힙 통계 출력
		{
			heap_stats();

		}

		else if (cmd == 'r') // 힙 초기화
		{
			for (int i = 0; i < MAX_BLOCKS; i++)
			{
				ptrs[i] = NULL;
			}
			heap_init();
			printf("힙 초기화 완료\n\n");

		}

		else if (cmd == 'c') // 화면 지우기
		{
			printf("\033[H\033[2J\033[3J");
			printf("명령어: m <slot> <size> (할당) / f <slot> (해제) / d (덤프) / s (통계) / r (힙 초기화) / c (화면 지우기) / q (종료)\n\n");

		}

		else if (cmd == 'q') // 종료
		{
			break;
		}
	}

	return 0;
}
