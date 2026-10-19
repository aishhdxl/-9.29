#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ========== 数据结构 ========== */

typedef struct {
    int rank;   /* 点数：3~15 对应 3~2，16=小王，17=大王 */
    int suit;   /* 花色：0=黑桃，1=红桃，2=梅花，3=方块，-1=王 */
} Card;

/* 排序规则：
   先按点数从小到大排（3 < 4 < ... < A < 2 < 小王 < 大王）
   同点数按花色从大到小排（黑桃 > 红桃 > 梅花 > 方块）*/

/* ========== 工具函数：中文名称转 rank/suit ========== */

const char *rank_names[] = {
    "", "", "", "3", "4", "5", "6", "7", "8", "9",
    "10", "J", "Q", "K", "A", "2", "小王", "大王"
};

const char *suit_names[] = { "黑桃", "红桃", "梅花", "方块" };

/* 解析一张牌，成功返回 1，失败返回 0
   支持的输入格式：
     黑桃A、红桃10、梅花K、方块3
     小王、大王
   （前后可带空格） */
int parse_card(const char *input, Card *c)
{
    char buf[64];
    int len;
    char *p = buf;

    /* 1. 去掉前后空白，复制到 buf */
    while (*input == ' ' || *input == '\t') input++;
    strncpy(buf, input, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    len = (int)strlen(buf);
    while (len > 0 && (buf[len-1] == ' ' || buf[len-1] == '\n' || buf[len-1] == '\r'))
        buf[--len] = '\0';
    if (len == 0) return 0;

    /* 2. 先判断是不是大小王 */
    if (strcmp(buf, "小王") == 0) {
        c->rank = 16;
        c->suit = -1;
        return 1;
    }
    if (strcmp(buf, "大王") == 0) {
        c->rank = 17;
        c->suit = -1;
        return 1;
    }

    /* 3. 普通牌：花色 + 点数
       花色两个字：黑桃、红桃、梅花、方块
       注意：UTF-8 下一个汉字占 3 字节，所以两个字共 6 字节，
       指针要向后移 6 位，不能是 4 位，否则会停在汉字中间。 */
    c->suit = -1;
    if (strncmp(buf, "黑桃", 6) == 0) { c->suit = 0; p += 6; }
    else if (strncmp(buf, "红桃", 6) == 0) { c->suit = 1; p += 6; }
    else if (strncmp(buf, "梅花", 6) == 0) { c->suit = 2; p += 6; }
    else if (strncmp(buf, "方块", 6) == 0) { c->suit = 3; p += 6; }

    if (c->suit == -1) return 0;  /* 花色不对 */

    /* 4. 解析点数 */
    if (strcmp(p, "10") == 0) { c->rank = 10; return 1; }

    if (strlen(p) == 1) {
        char ch = p[0];
        if (ch >= '3' && ch <= '9') { c->rank = ch - '0'; return 1; }
        if (ch == 'J' || ch == 'j') { c->rank = 11; return 1; }
        if (ch == 'Q' || ch == 'q') { c->rank = 12; return 1; }
        if (ch == 'K' || ch == 'k') { c->rank = 13; return 1; }
        if (ch == 'A' || ch == 'a') { c->rank = 14; return 1; }
        if (ch == '2')              { c->rank = 15; return 1; }
    }

    return 0;  /* 点数不对 */
}

/* 把一张牌转成中文字符串，方便输出 */
void card_to_str(const Card *c, char *out)
{
    if (c->rank == 16) { strcpy(out, "小王"); return; }
    if (c->rank == 17) { strcpy(out, "大王"); return; }
    sprintf(out, "%s%s", suit_names[c->suit], rank_names[c->rank]);
}

/* ========== 比较函数（给 qsort 用） ========== */

int compare_cards(const void *a, const void *b)
{
    const Card *ca = (const Card *)a;
    const Card *cb = (const Card *)b;

    if (ca->rank != cb->rank)
        return ca->rank - cb->rank;  /* 点数小的在前 */

    /* 同点数：花色大的在前（黑桃 0 > 红桃 1 > 梅花 2 > 方块 3）
       所以 suit 小的排前面 */
    return ca->suit - cb->suit;
}

/* ========== 检查是否重复 ========== */

int is_duplicate(Card *cards, int count, const Card *c)
{
    int i;
    for (i = 0; i < count; i++) {
        if (cards[i].rank == c->rank && cards[i].suit == c->suit)
            return 1;
    }
    return 0;
}

/* ========== 主函数 ========== */

int main(void)
{
    Card cards[54];   /* 最多 54 张 */
    int count = 0;
    int n;
    int i;
    char line[256];

    printf("===== 扑克牌排序程序 =====\n");
    printf("输入格式：黑桃A / 红桃10 / 梅花K / 方块3 / 小王 / 大王\n");
    printf("=========================\n\n");

    /* 1. 输入数量 */
    printf("请输入你要抽几张牌：");
    if (fgets(line, sizeof(line), stdin) == NULL) return 0;
    if (sscanf(line, "%d", &n) != 1 || n <= 0 || n > 54) {
        printf("输入不合法！数量必须是 1~54 之间的整数。\n");
        return 0;
    }

    /* 2. 逐张输入牌 */
    printf("\n请依次输入 %d 张牌（每张一行）：\n", n);
    for (i = 0; i < n; i++) {
        Card c;
        printf("第 %d 张：", i + 1);
        if (fgets(line, sizeof(line), stdin) == NULL) break;

        if (!parse_card(line, &c)) {
            printf("  [错误] 输入格式不对，这张牌不记录。\n");
            printf("         正确格式示例：黑桃A、红桃10、梅花Q、方块2、小王、大王\n");
            continue;
        }

        if (is_duplicate(cards, count, &c)) {
            char name[32];
            card_to_str(&c, name);
            printf("  [警告] 【%s】已经有了，重复的牌不记录。\n", name);
            continue;
        }

        cards[count++] = c;
    }

    if (count == 0) {
        printf("\n一张有效牌都没有，没法排。\n");
        return 0;
    }

    /* 3. 排序 */
    qsort(cards, count, sizeof(Card), compare_cards);

    /* 4. 输出 */
    printf("\n===== 排序结果（从小到大）=====\n");
    printf("共 %d 张有效牌：\n", count);
    for (i = 0; i < count; i++) {
        char name[32];
        card_to_str(&cards[i], name);
        printf("  %2d. %s\n", i + 1, name);
    }
    printf("================================\n");

    return 0;
}
