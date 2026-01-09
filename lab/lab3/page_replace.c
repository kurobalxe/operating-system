#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

// 定义页面置换算法类型
typedef enum {
    ALG_FIFO,
    ALG_LRU,
    ALG_OPT
} Algorithm;

// 配置参数
typedef struct {
    int page_size;          // 页面大小（指令数）
    int num_frames;         // 页框数量
    int total_instructions; // 总指令数
    int num_pages;          // 总页数
    Algorithm algorithm;    // 使用的算法
    int access_pattern;     // 访问模式：0-顺序，1-跳转，2-分支，3-循环，4-局部性随机
    int *access_sequence;   // 访问序列
    int seq_length;         // 访问序列长度
    float locality_factor;  // 局部性因子（0-1），越大局部性越强
} Config;

// 页框结构
typedef struct {
    int page_id;    // 页号
    int last_used;  // 最近使用时间（LRU用）
    int load_time;  // 加载时间（FIFO用）
    bool valid;     // 是否有效
} Frame;

// 模拟器结构
typedef struct {
    Config config;
    int *large_array;   // 大数组A模拟进程
    Frame *frames;      // 页框数组
    int *page_table;    // 页表
    int page_faults;    // 缺页次数
    int current_time;   // 当前时间
} Simulator;

// 函数声明
void init_config(Config *config);
void init_simulator(Simulator *sim, Config *config);
void generate_access_sequence(Simulator *sim);
void simulate(Simulator *sim);
void print_results(Simulator *sim);
void cleanup(Simulator *sim);

// 页面置换算法函数
int fifo_replace(Simulator *sim);
int lru_replace(Simulator *sim);
int opt_replace(Simulator *sim, int current_index);
int find_page_in_frames(Simulator *sim, int page_id);
void load_page(Simulator *sim, int page_id, int frame_index);

// 辅助函数
void print_frames(Simulator *sim);
void print_access_sequence(Simulator *sim);
void print_progress(int current, int total);

int main(int argc, char *argv[]) {
    // 设置随机种子
    srand(time(NULL));
    
    // 默认配置
    Config config = {
        .page_size = 10,
        .num_frames = 5,           // 增加默认页框数量
        .total_instructions = 2400,
        .algorithm = ALG_FIFO,
        .access_pattern = 3,       // 默认使用循环访问，局部性更好
        .seq_length = 1000,        // 减少访问序列长度
        .locality_factor = 0.8f    // 增加局部性
    };
    
    // 解析命令行参数
    int opt;
    while ((opt = getopt(argc, argv, "p:f:a:m:l:s:h")) != -1) {
        switch (opt) {
            case 'p':
                config.page_size = atoi(optarg);
                if (config.page_size <= 0) {
                    fprintf(stderr, "页大小必须为正数\n");
                    return 1;
                }
                break;
            case 'f':
                config.num_frames = atoi(optarg);
                if (config.num_frames <= 0) {
                    fprintf(stderr, "页框数量必须为正数\n");
                    return 1;
                }
                break;
            case 'a':
                if (strcmp(optarg, "fifo") == 0) {
                    config.algorithm = ALG_FIFO;
                } else if (strcmp(optarg, "lru") == 0) {
                    config.algorithm = ALG_LRU;
                } else if (strcmp(optarg, "opt") == 0) {
                    config.algorithm = ALG_OPT;
                } else {
                    fprintf(stderr, "未知的算法: %s\n", optarg);
                    fprintf(stderr, "可用算法: fifo, lru, opt\n");
                    return 1;
                }
                break;
            case 'm':
                config.access_pattern = atoi(optarg);
                if (config.access_pattern < 0 || config.access_pattern > 4) {
                    fprintf(stderr, "访问模式必须为0-4之间的数字\n");
                    fprintf(stderr, "0:顺序 1:跳转 2:分支 3:循环 4:局部性随机\n");
                    return 1;
                }
                break;
            case 'l':
                config.locality_factor = atof(optarg);
                if (config.locality_factor < 0 || config.locality_factor > 1) {
                    fprintf(stderr, "局部性因子必须在0-1之间\n");
                    return 1;
                }
                break;
            case 's':
                config.seq_length = atoi(optarg);
                if (config.seq_length <= 0) {
                    fprintf(stderr, "访问序列长度必须为正数\n");
                    return 1;
                }
                break;
            case 'h':
                printf("页面置换算法模拟器\n");
                printf("用法: %s [选项]\n", argv[0]);
                printf("选项:\n");
                printf("  -p <数字>   页面大小（默认: 10）\n");
                printf("  -f <数字>   页框数量（默认: 5）\n");
                printf("  -a <算法>   算法: fifo, lru, opt（默认: fifo）\n");
                printf("  -m <模式>   访问模式: 0-4（默认: 3）\n");
                printf("              0:顺序 1:跳转 2:分支 3:循环 4:局部性随机\n");
                printf("  -l <因子>   局部性因子 0-1（默认: 0.8）\n");
                printf("  -s <长度>   访问序列长度（默认: 1000）\n");
                printf("  -h          显示此帮助信息\n");
                return 0;
        }
    }
    
    // 验证配置
    if (config.total_instructions % config.page_size != 0) {
        fprintf(stderr, "总指令数必须是页面大小的整数倍\n");
        return 1;
    }
    
    if (config.seq_length > config.total_instructions) {
        fprintf(stderr, "访问序列长度不能超过总指令数\n");
        config.seq_length = config.total_instructions;
    }
    
    config.num_pages = config.total_instructions / config.page_size;
    
    printf("=== 页面置换算法模拟器配置 ===\n");
    printf("页面大小: %d 条指令\n", config.page_size);
    printf("页框数量: %d\n", config.num_frames);
    printf("总页数: %d\n", config.num_pages);
    printf("总指令数: %d\n", config.total_instructions);
    printf("访问序列长度: %d\n", config.seq_length);
    printf("局部性因子: %.2f\n", config.locality_factor);
    
    switch (config.algorithm) {
        case ALG_FIFO:
            printf("使用算法: FIFO\n");
            break;
        case ALG_LRU:
            printf("使用算法: LRU\n");
            break;
        case ALG_OPT:
            printf("使用算法: OPT\n");
            break;
    }
    
    switch (config.access_pattern) {
        case 0:
            printf("访问模式: 顺序\n");
            break;
        case 1:
            printf("访问模式: 跳转\n");
            break;
        case 2:
            printf("访问模式: 分支\n");
            break;
        case 3:
            printf("访问模式: 循环\n");
            break;
        case 4:
            printf("访问模式: 局部性随机\n");
            break;
    }
    printf("=============================\n\n");
    
    // 初始化模拟器
    Simulator sim;
    init_simulator(&sim, &config);
    
    // 生成访问序列
    generate_access_sequence(&sim);
    
    // 运行模拟
    simulate(&sim);
    
    // 打印结果
    print_results(&sim);
    
    // 清理资源
    cleanup(&sim);
    
    return 0;
}

void init_simulator(Simulator *sim, Config *config) {
    sim->config = *config;
    
    // 分配大数组A
    sim->large_array = (int*)malloc(config->total_instructions * sizeof(int));
    
    // 初始化大数组，填充随机值（1-1000）
    for (int i = 0; i < config->total_instructions; i++) {
        sim->large_array[i] = rand() % 1000 + 1;
    }
    
    // 分配页框数组
    sim->frames = (Frame*)malloc(config->num_frames * sizeof(Frame));
    for (int i = 0; i < config->num_frames; i++) {
        sim->frames[i].page_id = -1;
        sim->frames[i].last_used = -1;
        sim->frames[i].load_time = -1;
        sim->frames[i].valid = false;
    }
    
    // 分配页表（简化版，只记录页号到页框的映射）
    sim->page_table = (int*)malloc(config->num_pages * sizeof(int));
    for (int i = 0; i < config->num_pages; i++) {
        sim->page_table[i] = -1;  // -1表示不在内存中
    }
    
    // 分配访问序列
    sim->config.access_sequence = (int*)malloc(config->seq_length * sizeof(int));
    
    sim->page_faults = 0;
    sim->current_time = 0;
}

void generate_access_sequence(Simulator *sim) {
    Config *config = &sim->config;
    int *seq = config->access_sequence;
    int total_inst = config->total_instructions;
    
    switch (config->access_pattern) {
        case 0: // 顺序访问（强局部性）
            for (int i = 0; i < config->seq_length; i++) {
                seq[i] = i % total_inst;
            }
            break;
            
        case 1: // 跳转访问（中度局部性）
            {
                int current = 0;
                for (int i = 0; i < config->seq_length; i++) {
                    seq[i] = current;
                    // 70%概率顺序执行，30%概率跳转
                    if (rand() % 100 < 70) {
                        current = (current + 1) % total_inst;
                    } else {
                        int jump_distance = (rand() % 50) + 10; // 跳转10-60条指令
                        current = (current + jump_distance) % total_inst;
                    }
                }
            }
            break;
            
        case 2: // 分支访问（模拟if-else模式，较强局部性）
            {
                int current = 0;
                for (int i = 0; i < config->seq_length; i++) {
                    seq[i] = current;
                    // 80%概率在当前页面内移动
                    if (rand() % 100 < 80) {
                        current = (current + 1) % config->page_size + 
                                 (current / config->page_size) * config->page_size;
                    } else {
                        // 20%概率跳转到其他页面
                        int new_page = rand() % config->num_pages;
                        current = new_page * config->page_size + rand() % config->page_size;
                    }
                    current %= total_inst;
                }
            }
            break;
            
        case 3: // 循环访问（强局部性）
            {
                // 创建几个循环区域
                int num_loops = 5;
                int loop_size = config->page_size * 3; // 每个循环3个页面
                int loop_start[num_loops];
                for (int i = 0; i < num_loops; i++) {
                    loop_start[i] = rand() % (total_inst - loop_size);
                }
                
                int current_loop = 0;
                int pos_in_loop = 0;
                
                for (int i = 0; i < config->seq_length; i++) {
                    seq[i] = loop_start[current_loop] + pos_in_loop;
                    pos_in_loop = (pos_in_loop + 1) % loop_size;
                    
                    // 偶尔切换到其他循环
                    if (pos_in_loop == 0 && rand() % 100 < 30) {
                        current_loop = rand() % num_loops;
                    }
                }
            }
            break;
            
        case 4: // 局部性随机访问
        default:
            {
                int current = rand() % total_inst;
                for (int i = 0; i < config->seq_length; i++) {
                    seq[i] = current;
                    
                    // 根据局部性因子决定下一步
                    if ((float)rand() / RAND_MAX < config->locality_factor) {
                        // 高概率在附近访问（±20条指令范围内）
                        int delta = (rand() % 41) - 20; // -20 到 +20
                        current = (current + delta + total_inst) % total_inst;
                    } else {
                        // 低概率随机跳转
                        current = rand() % total_inst;
                    }
                }
            }
            break;
    }
}

int find_page_in_frames(Simulator *sim, int page_id) {
    for (int i = 0; i < sim->config.num_frames; i++) {
        if (sim->frames[i].valid && sim->frames[i].page_id == page_id) {
            return i; // 找到页框
        }
    }
    return -1; // 未找到
}

void load_page(Simulator *sim, int page_id, int frame_index) {
    // 更新页框
    sim->frames[frame_index].page_id = page_id;
    sim->frames[frame_index].last_used = sim->current_time;
    sim->frames[frame_index].load_time = sim->current_time;
    sim->frames[frame_index].valid = true;
    
    // 更新页表
    sim->page_table[page_id] = frame_index;
}

int fifo_replace(Simulator *sim) {
    int oldest_index = 0;
    int oldest_time = sim->frames[0].load_time;
    
    for (int i = 1; i < sim->config.num_frames; i++) {
        if (sim->frames[i].load_time < oldest_time) {
            oldest_time = sim->frames[i].load_time;
            oldest_index = i;
        }
    }
    
    // 从页表中删除旧页面的映射
    int old_page = sim->frames[oldest_index].page_id;
    if (old_page >= 0) {
        sim->page_table[old_page] = -1;
    }
    
    return oldest_index;
}

int lru_replace(Simulator *sim) {
    int lru_index = 0;
    int lru_time = sim->frames[0].last_used;
    
    for (int i = 1; i < sim->config.num_frames; i++) {
        if (sim->frames[i].last_used < lru_time) {
            lru_time = sim->frames[i].last_used;
            lru_index = i;
        }
    }
    
    // 从页表中删除旧页面的映射
    int old_page = sim->frames[lru_index].page_id;
    if (old_page >= 0) {
        sim->page_table[old_page] = -1;
    }
    
    return lru_index;
}

int opt_replace(Simulator *sim, int current_index) {
    // 找到未来最长时间不会使用的页面
    int *seq = sim->config.access_sequence;
    int seq_length = sim->config.seq_length;
    int page_size = sim->config.page_size;
    int farthest_index = 0;
    int farthest_distance = -1;
    
    for (int i = 0; i < sim->config.num_frames; i++) {
        if (!sim->frames[i].valid) {
            return i; // 有空闲页框
        }
        
        int page_id = sim->frames[i].page_id;
        int next_use = seq_length; // 默认未来不再使用
        
        // 查找页面在未来何时被使用
        for (int j = current_index + 1; j < seq_length; j++) {
            int accessed_page = seq[j] / page_size;
            if (accessed_page == page_id) {
                next_use = j;
                break;
            }
        }
        
        // 如果页面在未来不再使用，直接替换它
        if (next_use == seq_length) {
            // 从页表中删除旧页面的映射
            sim->page_table[page_id] = -1;
            return i;
        }
        
        // 更新最远使用的页面
        if (next_use > farthest_distance) {
            farthest_distance = next_use;
            farthest_index = i;
        }
    }
    
    // 从页表中删除旧页面的映射
    int old_page = sim->frames[farthest_index].page_id;
    sim->page_table[old_page] = -1;
    
    return farthest_index;
}

void simulate(Simulator *sim) {
    Config *config = &sim->config;
    int *seq = config->access_sequence;
    int page_size = config->page_size;
    
    printf("开始模拟...\n\n");
    
    for (int i = 0; i < config->seq_length; i++) {
        sim->current_time = i;
        int instruction_index = seq[i];
        int page_id = instruction_index / page_size;
        
        // 检查页面是否在内存中
        int frame_index = find_page_in_frames(sim, page_id);
        
        if (frame_index == -1) {
            // 缺页！
            sim->page_faults++;
            
            // 检查是否有空闲页框
            bool has_free_frame = false;
            for (int j = 0; j < config->num_frames; j++) {
                if (!sim->frames[j].valid) {
                    frame_index = j;
                    has_free_frame = true;
                    break;
                }
            }
            
            // 如果没有空闲页框，需要置换
            if (!has_free_frame) {
                switch (config->algorithm) {
                    case ALG_FIFO:
                        frame_index = fifo_replace(sim);
                        break;
                    case ALG_LRU:
                        frame_index = lru_replace(sim);
                        break;
                    case ALG_OPT:
                        frame_index = opt_replace(sim, i);
                        break;
                }
            }
            
            // 加载页面
            load_page(sim, page_id, frame_index);
        } else {
            // 页面命中，更新LRU信息
            sim->frames[frame_index].last_used = i;
        }
        
        // 每200条指令打印一次进度
        if (i % 200 == 0 && i > 0) {
            print_progress(i, config->seq_length);
        }
    }
    
    printf("\n模拟完成！\n\n");
}

void print_progress(int current, int total) {
    int percent = (current * 100) / total;
    printf("模拟进度: [");
    int bars = percent / 5;
    for (int i = 0; i < 20; i++) {
        if (i < bars) printf("=");
        else printf(" ");
    }
    printf("] %d%%\n", percent);
}

void print_frames(Simulator *sim) {
    printf("当前页框状态: ");
    for (int i = 0; i < sim->config.num_frames; i++) {
        if (sim->frames[i].valid) {
            printf("[框%d:页%d] ", i, sim->frames[i].page_id);
        } else {
            printf("[框%d:空] ", i);
        }
    }
    printf("\n");
}

void print_results(Simulator *sim) {
    printf("=== 模拟结果 ===\n");
    printf("总访问次数: %d\n", sim->config.seq_length);
    printf("缺页次数: %d\n", sim->page_faults);
    printf("命中次数: %d\n", sim->config.seq_length - sim->page_faults);
    printf("缺页率: %.2f%%\n", (float)sim->page_faults / sim->config.seq_length * 100);
    printf("命中率: %.2f%%\n", (float)(sim->config.seq_length - sim->page_faults) / sim->config.seq_length * 100);
    
    // 打印最终页框状态
    printf("\n最终页框状态:\n");
    print_frames(sim);
    
    // 打印算法比较（如果运行多次）
    printf("\n提示：\n");
    printf("1. 使用更多页框可降低缺页率: ./page_replacement -f 8\n");
    printf("2. 使用循环访问模式局部性更好: ./page_replacement -m 3\n");
    printf("3. 使用LRU算法通常效果较好: ./page_replacement -a lru\n");
    printf("4. 增加局部性因子: ./page_replacement -l 0.9\n");
}

void cleanup(Simulator *sim) {
    free(sim->large_array);
    free(sim->frames);
    free(sim->page_table);
    free(sim->config.access_sequence);
}
