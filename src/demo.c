//
// Created by bloo on 12/04/2016.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

enum suit {HEARTS, DIAMONDS, CLUBS, SPADES, NONE};
typedef enum suit Suit;

enum rank {HighCard, OnePair, TwoPairs, Three, Four, Straight, Flush, FullHouse, StraightFlush, RoyalFlush
};
typedef enum rank Rank;

enum state {None, Called, Raised, Checked, Bets, Allins, Folded, SB, BB};
typedef enum state State;

enum option {Call, Raise, Check, Bet, Allin, Fold};
typedef enum option Option;

struct card {
    Suit suit;
    int rank;
};
typedef struct card Card;

struct deck {
    Card cards[52];
    int card_index;
};
typedef struct deck Deck;

struct player {
    char name[20];
    int money;
    int bet;
    int status;
    Card hand[2];
    Card *max_hand;
    Rank rank;
    State state;
    int isBigBlind;
    int isSmallBlind;
    Option option;
    int isWinner;
};
typedef struct player Player;

struct table {
    int pot_money;
    int ante;
    int highest_bet; //the largest amount of total bet in one round
    Card card[5];
    int card_idx;
    int last_bet;
};
typedef struct table Table;

struct hand {
    Card card[7];
};
typedef struct hand Hand;

char* getSuit(Suit s) {
    switch(s){
        case HEARTS: return "\u2665";
        case DIAMONDS: return "\u2666";
        case CLUBS: return "\u2663";
        case SPADES: return "\u2660";
        case NONE: return NULL;
    }
    return NULL;
}

Deck* newDeck() {
    Deck* deck = malloc(sizeof(Deck));
    int k = 0;
    deck->card_index = 0;
    Card c;
    for (Suit i = HEARTS; i <= SPADES; i++) {
        for (int j = 1; j < 14; j++) {
            c.rank = j;
            c.suit = i;
            deck->cards[k] = c;
            k++;
            if (k == 52) {
                return deck;
            }
        }
    }
    return NULL;
}

void shuffleDeck(Deck* deck, int size) {
    srand((unsigned int) time(NULL));
    for (int i = size - 1; i > 0; i--) {
        int randIdx = rand() % i;
        Card temp = deck->cards[i];
        deck->cards[i] = deck->cards[randIdx];
        deck->cards[randIdx] = temp;
    }
}

Player* createPlayers(int num_player) {
    if (num_player >= 2 && num_player <= 10) {
        Player *players = malloc(sizeof(Player) * num_player);
        for (int i = 0; i < num_player; i++) {
            strcpy(players[i].name, "Player 0");
            players[i].status = 1;
            players[i].state = None;
            players[i].name[7] += i + 1;
            players[i].money = 5000;
            players[i].max_hand = malloc(sizeof(Card) * 5);
            players[i].bet = 0;
            players[i].isBigBlind = 0;
            players[i].isSmallBlind = 0;
        }
        return players;
    }
    return NULL;
}

void displayPlayerInfo(Player player) {
    printf("%s -- ", player.name);
    printf("Money: %i -- ", player.money);
    printf("State: %i --", player.state);
    if (player.status == 0) {
        printf("Status: Fold -- ");
    } else {printf("Status: Active -- ");}
    printf("Bet: %i -- ", player.bet);
    if (player.isBigBlind) {
        printf("Big Blind");
    } else if (player.isSmallBlind) {
        printf("Small Blind");
    }
}

Table * createTable() {
    Table * table = malloc(sizeof(Table));
    table->card_idx = 0;
    table->highest_bet = 0;
    return table;
}

void displayTableInfo(Table table) {
    printf("Pot: %i\n", table.pot_money);
    printf("Ante: %i\n", table.ante);
    printf("Highest bet: %i\n", table.highest_bet);
    printf("Last bet: %i\n", table.last_bet);
    if (table.card[0].rank != 0) {
        printf("Shared cards: ");
    }
    for (int i = 0; i <= table.card_idx; i++) {
        if (table.card[i].rank == 0) {
            break;
        }
        printf("%s %i; ", getSuit(table.card[i].suit), table.card[i].rank);
        if (i == table.card_idx) {
            printf("\n");
        }
    }
    printf("\n");
}

void dealStartingHand(Player*players, Deck* deck, int num_player) {
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < num_player; i++) {
            if (players[i].money <= 0) {
                continue;
            }
            players[i].hand[j] = deck->cards[deck->card_index];
            deck->card_index++;
        }
    }
}

void dealSharedCards(Table *table, Deck* deck, int time){
    //1st time: deal 3 cards: largest idx = 2
    //2nd time: deal 1 card:  largest idx = 3
    //3rd time: deal 1 card:  largest idx = 4 --> largest idx = time + 1
    for (; table->card_idx <= time + 1; table->card_idx++) {
        table->card[table->card_idx] = deck->cards[deck->card_index];
        deck->card_index++;
    }
};

Hand * createHands(Player *players, Table *table, int num_player) {
    Hand * hands = malloc(sizeof(Hand) * num_player);
    for (int i = 0; i < num_player; i++) {
        for (int j = 0; j < 2; j++) {
            hands[i].card[j] = players[i].hand[j];
        }
    }

    for (int i = 0; i < num_player; i++) {
        for (int j = 0; j < 5; j++) {
            hands[i].card[j + 2] = table[0].card[j];
        }
    }
    return hands;
}

void swapCards(Hand* hand, int playerIdx, int fstIdx, int secIdx) {
    Card temp = hand[playerIdx].card[fstIdx];
    hand[playerIdx].card[fstIdx] = hand[playerIdx].card[secIdx];
    hand[playerIdx].card[secIdx] = temp;
}

void sortHands(Hand *hands, int num_player) {
    for (int i = 0; i < num_player; i++) {
        int idx = 0; //if there is an Ace, the ace card will swap with the card that has this idx
        for (int j = 0; j < 7; j++) {
            if (hands[i].card[j].rank == 1) {
                swapCards(hands, i, idx, j);
                idx++;
            }
        }
    }
    for (int i = 0; i < num_player; i++) {
        Card max_rank_card;
        for (int j = 0; j < 7; j++) {
            max_rank_card.rank = hands[i].card[j].rank;
            if (max_rank_card.rank == 1) { //have checked Ace already, do not touch Ace again
                continue;
            };
            for (int k = j+1; k < 7; k++) {
                if (hands[i].card[k].rank > max_rank_card.rank) {
                    max_rank_card.rank = hands[i].card[k].rank;
                    swapCards(hands, i, j, k);
                }
            }
        }
    }
}

int searchHandRank(Hand hand, int rank) {
    for (int i = 0; i < 7; i++) {
        if (hand.card[i].rank == rank) {
            return 1;
        }
    }
    return 0;
}

Card searchCard(Hand hand, int rank) {
    Card temp = {HEARTS};
    for (int i = 0; i < 7; i++) {
        if (hand.card[i].rank == rank) {
            return hand.card[i];
        }
    }
    return temp;
}

Card isHighestCard(Hand hand) {
    Card maxCard;
    maxCard.rank = 1;
    for (int i = 0; i < 7; i++){
        if (hand.card[i].rank == 1) {
            maxCard = hand.card[i];
            return maxCard;
        } else if (hand.card[i].rank > maxCard.rank) {
            maxCard = hand.card[i];
        }
    }
    return maxCard;
};

//the hand must be sorted before checking
void isHighCard(Hand hand, Player* player) {
    for (int i = 0; i < 5; i++) {
        player->max_hand[i] = hand.card[i];
    }
    player->rank = HighCard;
}

int isPair(Hand hand, Player* player){
    int exist= 0;
    int idx = 2;
    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 7; j++) {
            if(hand.card[i].rank == hand.card[j].rank) {
                exist = hand.card[i].rank;
                player->max_hand[0] = hand.card[i];
                player->max_hand[1] = hand.card[j];
                player->rank = OnePair;
                for (int l = 0; l < 7; l++) {
                    if(hand.card[l].rank != exist) {
                        player->max_hand[idx] = hand.card[l];
                        idx++;
                    }
                    if (idx == 5) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int is2Pair(Hand hand, Player*player) {
    int count = 0;
    int exist[2] = {0, 0};
    int idx = 0;
    int ext_idx = 0;

    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 7; j++) {
            if(hand.card[i].rank == hand.card[j].rank && hand.card[i].rank != exist[0]) {
                exist[ext_idx] = hand.card[i].rank;
                player->max_hand[idx] = hand.card[i];
                idx++;
                player->max_hand[idx] = hand.card[j];
                count++;
                ext_idx++;
                idx++;
            }
            if (count == 2) {
                for (int k = 0; k < 7; k++) {
                    if (hand.card[k].rank != exist[0] && hand.card[k].rank != exist[1]) {
                        player->max_hand[idx] = hand.card[k];
                        player->rank = TwoPairs;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int is3OfAKind(Hand hand, Player*player) {
    int exist = 0;
    int idx = 3;
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 6; j++) {
            for (int k = j + 1; k < 7; k++) {
                if (hand.card[i].rank == hand.card[j].rank &&
                    hand.card[i].rank == hand.card[k].rank &&
                    hand.card[i].rank != exist) {
                    player->max_hand[0] = hand.card[i];
                    player->max_hand[1] = hand.card[j];
                    player->max_hand[2] = hand.card[k];
                    exist = hand.card[i].rank;
                    for (int l = 0; l < 7; l++) {
                        if (hand.card[l].rank != exist) {
                            player->max_hand[idx] = hand.card[l];
                            idx++;
                        }
                        if (idx == 5) {
                            player->rank = Three;
                            return 1;
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int is4OfAKind(Hand hand, Player*player) {
    int exist = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            for (int k = j + 1; k < 6; k++) {
                for (int l = k + 1; l < 7; l++) {
                    if (hand.card[i].rank == hand.card[j].rank &&
                        hand.card[i].rank == hand.card[k].rank &&
                        hand.card[i].rank == hand.card[l].rank) {
                        exist = hand.card[i].rank;
                        player->max_hand[0] = hand.card[i];
                        player->max_hand[1] = hand.card[j];
                        player->max_hand[2] = hand.card[k];
                        player->max_hand[3] = hand.card[l];
                        for (int m = 0; m < 7; m++) {
                            if(hand.card[m].rank != exist) {
                                player->max_hand[4] = hand.card[m];
                                player->rank = Four;
                                return 1;
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}

int isFullHouse(Hand hand, Player*player) {
    int exist = 0;
    int count = 0;
    if (is3OfAKind(hand, player)){
        for (int i = 0; i < 5; i++) {
            for (int j = i + 1; j < 6; j++) {
                for (int k = j + 1; k < 7; k++) {
                    if (hand.card[i].rank == hand.card[j].rank &&
                        hand.card[i].rank == hand.card[k].rank &&
                        hand.card[i].rank != exist) {
                        exist = hand.card[i].rank;
                    }
                }
            }
        }
        for (int i = 0; i < 6; i++) {
            for (int j = i + 1; j < 7; j++) {
                if(hand.card[i].rank == hand.card[j].rank && hand.card[i].rank != exist) {
                    player->max_hand[3] = hand.card[i];
                    player->max_hand[4] = hand.card[j];
                    count++;
                }
                if (count >= 1) {
                    player->rank = FullHouse;
                    return 1;
                }
            }
        }
    }
    return 0;
}

//the hand must be sorted before checking
int isStraight(Hand hand, Player*player) {
    for (int j = 0; j < 3; j++) {
        int check = 0;
        Card temp = hand.card[j];
        player->max_hand[check] = searchCard(hand, temp.rank);
        check++;
        if (temp.rank == 1) {
            for (int k = 13; k >= 10; k--) {
                if (searchHandRank(hand, k)) {
                    player->max_hand[check] = searchCard(hand, k);
                    check++;
                } else {
                    break;
                }
            }
            if (check == 5) {
                player->rank = Straight;
                return 1;
            }
            check = 0; //if the straight is 5 4 3 2 1, Ace will be placed at the end of max_hand
            for (int k = 5; k >= 2; k--) {
                if (searchHandRank(hand, k)) {
                    player->max_hand[check] = searchCard(hand, k);
                    check++;
                } else {
                    break;
                }
            }
            if (check == 4) {
                player->max_hand[check] = searchCard(hand, 1);
                player->rank = Straight;
                return 1;
            }
        }

        check = 1;
        for (int k = 1; k < 5; k++) {
            if (temp.rank - k > 0 && searchHandRank(hand, temp.rank - k)) {
                player->max_hand[check] = searchCard(hand, temp.rank - k);
                check++;
            };
        }
        if (check == 5) {
            player->rank = Straight;
            return 1;
        }
    }
    return 0;
}

//the hand must be sorted before checking
int isFlush(Hand hand, Player* player) {
    for (Suit suit = HEARTS; suit <= SPADES; suit++) {
        int count = 0; //count number of cards that have the same suit
        for (int j = 0; j < 7; j++) {
            if (hand.card[j].suit == suit && hand.card[j].rank > 0) {
                player->max_hand[count] = hand.card[j]; //add card to max_hand
                count++;
                if (count == 5) {
                    player->rank = Flush;
                    return 1;
                }
            }
        }
    }
    return 0;
}

//the hand must be sorted before checking
int isStraightFlush(Hand hand, Player* player) {
    for (Suit suit = HEARTS; suit <= SPADES; suit++) {
        Hand* temp = malloc(sizeof(Hand));
        int count = 0; //count number of cards that have the same suit
        for (int j = 0; j < 7; j++) {
            if (hand.card[j].suit == suit && hand.card[j].rank > 0) {
                temp->card[count] = hand.card[j];
                count++;
            }
        }

        if (count >= 5 && isStraight(*temp, player)) {
            free(temp);
            player->rank = StraightFlush;
            return 1;
        }
        free(temp);
    }
    return 0;
}

int isRoyalFlush(Hand hand, Player *player) {
    for (Suit suit = HEARTS; suit <= SPADES; suit++) {
        Hand* temp = malloc(sizeof(Hand));
        int count = 0; //count number of cards that have the same suit
        int check = 0;
        for (int j = 0; j < 7; j++) {
            if (hand.card[j].suit == suit && hand.card[j].rank > 0) {
                temp->card[count] = hand.card[j];
                count++;
            }
        }
        if (count >= 5) {
            if (searchHandRank(*temp, 1)) {
                player->max_hand[check] = searchCard(*temp, 1); //add card to max_hand
                check++;
            }
            for (int i = 13; i >= 10; i--) {
                if (searchHandRank(*temp, i)) {
                    player->max_hand[check] = searchCard(*temp, i); //add card to max_hand
                    check++;
                }
            }
            if (check == 5) {
                free(temp);
                player->rank = RoyalFlush;
                return 1;
            }
        }
        free(temp);
    }
    return 0;
}

int scanInput(int num_option) {
    int n;
    int input = 0;
    char c;
    int the_end = 0;

    while (!the_end) {
        printf("Enter input: ");
        n = scanf("%d", &input);
        if (n == 0) {
            c = getchar();
            while (c != '\n') {
                c = getchar();
            }
            printf("Invalid input\n");
        } else if (input >= 1 && input <= num_option){
            the_end = 1;

        } else {
            printf("Invalid input\n");
        }
    }
    printf("\n");
    return input;
}

void updateMoney(Player *player, Table * table, int money) {
    player->money = player->money - money;
    player->bet += money;
    table->pot_money += money;
}

void displayRangeMoney(int min, int max) {
    printf("The money should be between %i and %i\n", min, max);
}

int inputMoney(int min, int max) {
    int n;
    int input = 0;
    char c;
    int the_end = 0;

    while (!the_end) {
        printf("Enter money: ");
        n = scanf("%d", &input);
        if (n == 0) {
            c = getchar();
            while (c != '\n') {
                c = getchar();
            }
            printf("Invalid input\n");
        } else if (input >= min && input <= max){
            the_end = 1;
        } else {
            printf("Invalid input\n");
        }
    }
    printf("\n");
    return input;
}

int isAllin(Player player, Table table) {
    return player.money < table.highest_bet - player.bet;
}

int isCallRaise(Player player, Table table) {
    return player.bet < table.highest_bet;
}

int isCheckBet(Player player, Table table) {
    return player.bet == table.highest_bet;
}

void allin(Player *player, Table * table) {
    if (isAllin(*player, *table)) {
        player->option = Allin;
        updateMoney(player, table, player->money);
        player->state = Allins;
    }
}

void call(Player *player, Table * table) {
    if (isAllin(*player, *table)) {
        allin(player, table);
    } else if (player->money >= table->highest_bet - player->bet) {
        player->option = Call;
        updateMoney(player, table, table->highest_bet - player->bet);
        player->state = Called;
    }
}

void raisePoker(Player *player, Table *table, int money) {
    int call_money = table->highest_bet - player->bet;
    if (isAllin(*player, *table)) {
        allin(player, table);
    } else if (player->money >= money && money >= call_money + table->last_bet ) {
        player->option = Raise;
        updateMoney(player, table, money);
        player->state = Raised;
    }
}

void check(Player * player) {
    player->option = Check;
    player->state = Checked;
}

void bet(Player * player, Table * table, int money) {
    if (isAllin(*player, *table)) {
        allin(player, table);
    } else if (money >= table->ante * 2 && player->money >= money) {
        player->option = Bet;
        updateMoney(player, table, money);
        player->state = Bets;
    }
}

void fold(Player * player) {
    player->status = 0;
    player->state = Folded;
}

int minMoney(Player player, Table table) {
    if (isCallRaise(player, table)) {
        return table.highest_bet - player.bet + table.last_bet;
    } else if (isCheckBet(player, table)) {
        return table.ante * 2;
    }
    return 0;
}

void displayOption(Player player, Table table) {
    printf("Choose option:\n");
    if (isCallRaise(player, table)) {
        if (isAllin(player, table)) {
            printf("1. Allin");
        } else if(player.money < table.highest_bet - player.bet + table.last_bet){
            printf("1. Call\n");
            printf("2. Allin\n");
        } else {
            printf("1. Call\n");
            printf("2. Raise\n");
        }
    } else if (isCheckBet(player, table)) {
        printf("1. Check\n");
        if (player.money >= table.ante * 2) {
            printf("2. Bet\n");
        } else {
            printf("2. Allin\n");
        }
    }
    printf("3. Fold\n");
}

void runOption(Player * player, Table * table, int option, int money) {
    if (isCallRaise(*player, *table)) {
        if (isAllin(*player, *table)) {
            if (option == 1) {
                allin(player, table);
            }
        } else if (player->money < table->highest_bet - player->bet + table->last_bet) {
            if (option == 1) {
                call(player, table);
            } else if (option == 2) {
                allin(player, table);
            }
        } else {
            if (option == 1) {
                call(player, table);
            } else if (option == 2) {
                raisePoker(player, table, money);
                table->last_bet = money;
            }
        }
    } else if (isCheckBet(*player, *table)) {
        if (option == 1) {
            check(player);
        } else {
            if (player->money >= table->ante *2) {
                if (option == 2) {
                    bet(player, table, money);
                    table->last_bet = money;
                }
            } else {
                allin(player, table);
            }
        }
    }
    if (option == 3) {
        fold(player);
    }
}

void checkHandRanking(Hand * hand, Player * player) {
    sortHands(hand, 1);
    if (isRoyalFlush(*hand, player)) {
    } else if (isStraightFlush(*hand, player)) {
    } else if (is4OfAKind(*hand, player)) {
    } else if (isFullHouse(*hand, player)) {
    } else if (isFlush(*hand, player)) {
    } else if (isStraight(*hand, player)) {
    } else if (is3OfAKind(*hand, player)) {
    } else if (is2Pair(*hand, player)) {
    } else if (isPair(*hand, player)) {
    } else {
        isHighCard(*hand, player);
    }
}

int aggrAIround0(Player *ai, Table *table) {
    if (ai->hand[0].rank == ai->hand[1].rank ||
        (ai->hand[0].rank >= 10 && ai->hand[1].rank >= 10) ||
        ai->hand[0].rank == 1 ||
        ai->hand[1].rank == 1) {
        if (isCallRaise(*ai, *table)) {
            int money = minMoney(*ai, *table);
            if (ai->money >= money * 3) {
                raisePoker(ai, table, (int) (money * 1.5));
                return 2;
            }
        } else if (isCheckBet(*ai, *table)) {
            int money = minMoney(*ai, *table);
            if (ai->money >= money * 3) {
                bet(ai, table, (int) (money * 1.5));
                return 2;
            }
        }
    }
    if (abs(ai->hand[0].rank - ai->hand[1].rank) > 5 && ai->hand[0].suit != ai->hand[1].suit) {
        if (ai->hand[0].rank < 11 && ai->hand[1].rank < 11) {
            fold(ai);
            return 3;
        }
    }
    if (isCallRaise(*ai, *table)) {
        if (ai->money >= minMoney(*ai, *table)) {
            call(ai, table);
            return 1;
        }
    } else if (isCheckBet(*ai, *table)) {
        check(ai);
        return 1;
    }
    fold(ai);
    return 3;

}

int aggrAIrounds(Player *ai, Table *table) {
    Hand * temp = malloc(sizeof(Hand));
    temp->card[0] = ai->hand[0];
    temp->card[1] = ai->hand[1];
    for (int i = 0; i < table->card_idx; i++) {
        temp->card[i+2] = table->card[i];
    }
    if (table->card_idx >= 3) {
        if (table->card_idx == 3) {
            temp->card[5].rank = -1;
            temp->card[5].suit = NONE;
        }
        temp->card[6].rank = -2;
        temp->card[6].suit = NONE;
    }
    checkHandRanking(temp, ai);

    int money = minMoney(*ai, *table);
    if (ai->rank > 0) {
        if (ai->rank >= 2) {
            if (isCallRaise(*ai, *table)) {
                raisePoker(ai, table, ai->money);
                return 2;
            } else if (isCheckBet(*ai, *table)) {
                bet(ai, table, ai->money);
                return 2;
            }
        }
        if (ai->money >= money * 3) {
            if (isCallRaise(*ai, *table)) {
                raisePoker(ai, table, (int) (money * 1.5));
                return 2;
            } else if (isCheckBet(*ai, *table)) {
                bet(ai, table, (int)(money * 1.5));
                return 2;
            }
        }
        if (isCallRaise(*ai, *table)) {
            call(ai, table);
            return 1;
        } else if (isCheckBet(*ai, *table)) {
            check(ai);
            return 1;
        }
    }

    if (ai->money >= money * 2) {
        if (isCallRaise(*ai, *table)) {
            call(ai, table);
            return 1;
        } else if (isCheckBet(*ai, *table)) {
            check(ai);
            return 1;
        }

    }
    fold(ai);

    free(temp);
    return 3;
}

int aggrAI(Player *ai, Table *table, int roundIdx) {
    int input;
    if (roundIdx == 0) {
        input = aggrAIround0(ai, table);
    } else {
        input = aggrAIrounds(ai, table);
    }
    return input;
}

int consAIround0(Player *ai, Table * table) {
    if (ai->hand[0].rank == 1 || ai->hand[1].rank == 1 ||
        (ai->hand[0].rank == ai->hand[1].rank && (ai->hand[0].rank == 1 || ai->hand[0].rank > 6)) ||
        (ai->hand[0].rank > 7 && ai->hand[1].rank > 7)) {
        if (isCallRaise(*ai, *table)) {
            call(ai, table);
            return 1;
        } else if (isCheckBet(*ai, *table)) {
            check(ai);
            return 1;
        }
    }
    fold(ai);
    return 3;
}

int consAIrounds(Player *ai, Table * table) {
    Hand * temp = malloc(sizeof(Hand));
    temp->card[0] = ai->hand[0];
    temp->card[1] = ai->hand[1];
    for (int i = 0; i < table->card_idx; i++) {
        temp->card[i+2] = table->card[i];
    }
    if (table->card_idx >= 3) {
        if (table->card_idx == 3) {
            temp->card[5].rank = -1;
            temp->card[5].suit = NONE;
        }
        temp->card[6].rank = -2;
        temp->card[6].suit = NONE;
    }
    checkHandRanking(temp, ai);
    if (ai->rank > 1) {
        if (isCallRaise(*ai, *table)) {
            call(ai, table);
            return 1;
        } else if (isCheckBet(*ai, *table)) {
            check(ai);
            return 1;
        }
    }
    fold(ai);
    return 3;
}

int consAI (Player *ai, Table *table, int roundIdx) {
    int input;
    if (roundIdx == 0) {
        input = consAIround0(ai, table);
    } else {
        input = consAIrounds(ai, table);
    }
    return input;
}

int turn(Player *player, Table * table, int roundIdx, int playerIdx) {
    int input;
    printf("\n\n%s's Turn\n", player->name);

    //print cards in hand
    for (int j = 0; j < 2; j++) {
        printf("%s %i; ", getSuit(player->hand[j].suit), player->hand[j].rank);
    }
    printf("\n\n");

    //TODO: add AI here
    if (playerIdx != 0) {
        if (playerIdx % 2  == 0) {
            input = aggrAI(player, table, roundIdx);
        } else {
        input = consAI(player, table, roundIdx);
        }
    } else {
        // Let user choose option
        displayOption(*player, * table);
        input = scanInput(3);

        // Let user input money if they choose raisePoker or bet
        int money = 0;
        int isRaiseOrBet = 0;
        if (isCallRaise(*player, *table)) {
            int call_money = table->highest_bet - player->bet;
            if (player->money >= call_money + table->last_bet) {
                if (input == 2) {
                    isRaiseOrBet = 1;
                }
            }
        } else if (isCheckBet(*player, *table)) {
            if (player->money >= table->ante * 2) {
                if (input == 2) {
                    isRaiseOrBet = 1;
                }
            }
        }
        if (isRaiseOrBet == 1) {
            int min = minMoney(*player, *table);
            displayRangeMoney(min, player->money);
            money = inputMoney(min, player->money);
        }

        //execute the option
        runOption(player, table, input, money);
    }

    //Update highest_bet
    if (player->bet > table->highest_bet) {
        table->highest_bet = player->bet;
    }
    printf("\nHigh bet: %i\n", table->highest_bet);
    printf("\n------End%sTurn-----\n", player->name);
    return input;
}

void save(Player * player, Table * table, Deck * deck, int num_player, int round_index, int player_index,
          int countCheck, int countAllin, int countCall, int is_1st_bet){
    FILE *save_point;
    save_point = fopen("../src/game.txt", "w+");
    if (save_point != NULL) {
        fprintf(save_point, "Round index\n%i\n----------\n", round_index);
        fprintf(save_point, "Player index\n%i\n----------\n", player_index);
        fprintf(save_point, "Number of player\n%i\n----------\n", num_player);
        fprintf(save_point, "Deck\n");
        for (int i = 0; i < 52; i++) {
            fprintf(save_point, "%i %i\n", deck->cards[i].suit, deck->cards[i].rank);
        }
        fprintf(save_point, "----------\nCount Check\n%d\n", countCheck);
        fprintf(save_point, "----------\nCount Allin\n%d\n", countAllin);
        fprintf(save_point, "----------\nCount Call\n%d\n", countCall);
        fprintf(save_point, "----------\nIs First Bet\n%d\n", is_1st_bet);
        fprintf(save_point, "----------\nTable\n");
        fprintf(save_point, "%i\n%i\n%i\n%i\n", table->pot_money, table->ante, table->highest_bet, table->last_bet);

        fprintf(save_point, "Shared cards\n");
        if (round_index == 0) {
            fprintf(save_point, "\n\n\n\n\n");
        } else if (round_index == 1) {
            for (int j = 0; j < 3; j++) {
                fprintf(save_point, "%i %i\n", table->card[j].suit, table->card[j].rank);
            }
            fprintf(save_point, "\n\n");
        } else if (round_index == 2) {
            for (int j = 0; j < 4; j++) {
                fprintf(save_point, "%i %i\n", table->card[j].suit, table->card[j].rank);
            }
            fprintf(save_point, "\n");
        } else if (round_index == 3) {
            for (int j = 0; j < 5; j++) {
                fprintf(save_point, "%i %i\n", table->card[j].suit, table->card[j].rank);
            }
        } else {
            fprintf(save_point, "wtf\n");
        }
        fprintf(save_point, "----------\nPlayers\n");
        for (int i = 0; i < num_player; i++) {
            fprintf(save_point, "Player %i:\n%i\n%i\n%i\n%i %i\n%i %i\n%i\n%i\n", i + 1,
                    player[i].money, player[i].bet, player[i].state,
                    player[i].hand[0].suit, player[i].hand[0].rank,
                    player[i].hand[1].suit, player[i].hand[1].rank,
                    player[i].isSmallBlind, player[i].isBigBlind);
        }
        fclose(save_point);
    } else {
        printf("cannot open file\n");
    }
}

int loadNumPlayer() {
    FILE * save;
    char line [20];
    int i = 0, num_player = 0; // i: count line
    save = fopen("../src/game.txt", "r");
    if (save != NULL) {
        while (fgets(line, 20, save) != NULL) {
            i++;
            // Load number of players
            if (i == 8) {
                num_player = atoi(line);
                printf("Number of players: %i\n", num_player);
                break;
            }
        }
        return num_player;
        fclose(save);
    } else {
        printf("Can't open file\n");
    }
    return 0;
}

void loadRoundInfo(int countCheck, int countAllin, int countCall, int is_1st_bet) {
    FILE * save;
    char line [20];
    char *s;
    int i = 0; // i: count line
    save = fopen("../src/game.txt", "r");
    if (save != NULL) {
        while (fgets(line, 20, save) != NULL) {
            i++;
            // Load count check
            if (i == 65) {
                countCheck = atoi(line);
                printf("Count Check: %i\n", countCheck);
            } else if (i == 68) {
                countAllin = atoi(line);
                printf("Count Allin: %i\n", countAllin);
            } else if (i == 71) {
                countCall = atoi(line);
                printf("Count Call: %i\n", countCall);
            } else if (i == 74) {
                is_1st_bet = atoi(line);
                printf("Is First Bet: %i\n", is_1st_bet);
                break;
            }
        }
        fclose(save);
    } else {
        printf("Can't open file\n");
    }
}

void load(Table * table, Deck * deck, Player * players, int * round_index) {
    FILE * save;
    char line [20];
    char *s;
    int i = 0, j = 0, l = 0, m = 0, num_player = 0; // i: count line, j: deck size, l: share card size
    save = fopen("../src/game.txt", "r");
    if (save != NULL) {
        while (fgets(line, 20, save) != NULL) {
            i++;

            // Load round index
            if (i == 2) {
                *round_index = atoi(line);
                printf("Round index: %i\n", *round_index);
            }

            // Load number of Player
            if (i == 8) {
                num_player = atoi(line);
                printf("Number of player: %i\n", num_player);
            }

            // Load deck
            if (i >= 11 && i <= 62) {
                s = strtok(line, " ");
                int k = 0;
                while (s != NULL) {
                    k++;
                    if (j < 52) {
                        if (k == 1) {
                            if (atoi(s) == 0) {
                                deck->cards[j].suit = HEARTS;
                            } if (atoi(s) == 1) {
                                deck->cards[j].suit = DIAMONDS;
                            } if (atoi(s) == 2) {
                                deck->cards[j].suit = CLUBS;
                            } else if (atoi(s) == 3) {
                                deck->cards[j].suit = SPADES;
                            }
                        } else if (k == 2) {
                            deck->cards[j].rank = atoi(s);
                        }
                    }
                    s = strtok(NULL, " ");
                }
                j++;
            }

            // Load table
            if (i >= 77 && i <= 80) {
                if (i == 77) {
                    table->pot_money = atoi (line);
                }
                if (i == 78) {
                    table->ante = atoi (line);
                }
                if (i == 79) {
                    table->highest_bet = atoi (line);
                }
                if (i == 80) {
                    table->last_bet = atoi (line);
                }
            }

            // Load shared cards
            if (*round_index != 0) {
                if (*round_index == 1) {
                    table->card_idx = 3;

                    if (i >= 82 && i <= 84){
                        s = strtok(line, " ");
                        int k = 0;
                        while (s != NULL) {
                            k++;
                            if (l < 3) {
                                if (k == 1) {
                                    if (atoi(s) == 0) {
                                        table->card[l].suit = HEARTS;
                                    } if (atoi(s) == 1) {
                                        table->card[l].suit = DIAMONDS;
                                    } if (atoi(s) == 2) {
                                        table->card[l].suit = CLUBS;
                                    } else if (atoi(s) == 3) {
                                        table->card[l].suit = SPADES;
                                    }
                                } else if (k == 2) {
                                    table->card[l].rank = atoi(s);
                                }
                            }
                            s = strtok(NULL, " ");
                        }
                        l++;
                    }
                } else if (*round_index == 2) {
                    table->card_idx = 4;

                    if (i >= 82 && i <= 85){
                        s = strtok(line, " ");
                        int k = 0;
                        while (s != NULL) {
                            k++;
                            if (l < 4) {
                                if (k == 1) {
                                    if (atoi(s) == 0) {
                                        table->card[l].suit = HEARTS;
                                    } if (atoi(s) == 1) {
                                        table->card[l].suit = DIAMONDS;
                                    } if (atoi(s) == 2) {
                                        table->card[l].suit = CLUBS;
                                    } else if (atoi(s) == 3) {
                                        table->card[l].suit = SPADES;
                                    }
                                } else if (k == 2) {
                                    table->card[l].rank = atoi(s);
                                }
                            }
                            s = strtok(NULL, " ");
                        }
                        l++;
                    }
                } else if (*round_index == 3) {
                    table->card_idx = 5;

                    if (i >= 82 && i <= 86){
                        s = strtok(line, " ");
                        int k = 0;
                        while (s != NULL) {
                            k++;
                            if (l < 5) {
                                if (k == 1) {
                                    if (atoi(s) == 0) {
                                        table->card[l].suit = HEARTS;
                                    } if (atoi(s) == 1) {
                                        table->card[l].suit = DIAMONDS;
                                    } if (atoi(s) == 2) {
                                        table->card[l].suit = CLUBS;
                                    } else if (atoi(s) == 3) {
                                        table->card[l].suit = SPADES;
                                    }
                                } else if (k == 2) {
                                    table->card[l].rank = atoi(s);
                                }
                            }
                            s = strtok(NULL, " ");
                        }
                        l++;
                    }
                }
            }

            // Load player 1
            if (i >= 90 && i <= 96) {
                if (i == 90) {
                    players[0].money = atoi(line);
                }
                if (i == 91) {
                    players[0].bet = atoi(line);
                }
                if (i == 92) {
                    switch (atoi(line)) {
                        case 1: players[0].state = Called;
                            break;
                        case 2: players[0].state = Raised;
                            break;
                        case 3: players[0].state = Checked;
                            break;
                        case 4: players[0].state = Bets;
                            break;
                        case 5: players[0].state = Allins;
                            break;
                        case 6: players[0].state = Folded;
                            break;
                        case 7: players[0].state = SB;
                            break;
                        case 8: players[0].state = BB;
                            break;
                        default: players[0].state = None;
                    }
                }
                if (i >= 93 && i <= 94) {
                    s = strtok(line, " ");
                    int k = 0;
                    while (s != NULL) {
                        k++;
                        if (m < 2) {
                            if (k == 1) {
                                if (atoi(s) == 0) {
                                    players[0].hand[m].suit = HEARTS;
                                } if (atoi(s) == 1) {
                                    players[0].hand[m].suit = DIAMONDS;
                                } if (atoi(s) == 2) {
                                    players[0].hand[m].suit = CLUBS;
                                } else if (atoi(s) == 3) {
                                    players[0].hand[m].suit = SPADES;
                                }
                            } else if (k == 2) {
                                players[0].hand[m].rank = atoi(s);
                            }
                        }
                        s = strtok(NULL, " ");
                    }
                    m++;
                }
                if (i == 95) {
                    players[0].isSmallBlind = atoi(line);
                }
                if (i == 96) {
                    players[0].isBigBlind = atoi(line);
                }
            }

            // Load player 2
            if (i >= 98 && i <= 104) {
                m = 0;
                if (i == 98) {
                    players[1].money = atoi(line);
                }
                if (i == 99) {
                    players[1].bet = atoi(line);
                }
                if (i == 100) {
                    switch (atoi(line)) {
                        case 1: players[1].state = Called;
                            break;
                        case 2: players[1].state = Raised;
                            break;
                        case 3: players[1].state = Checked;
                            break;
                        case 4: players[1].state = Bets;
                            break;
                        case 5: players[1].state = Allins;
                            break;
                        case 6: players[1].state = Folded;
                            break;
                        case 7: players[1].state = SB;
                            break;
                        case 8: players[1].state = BB;
                            break;
                        default: players[1].state = None;
                    }
                }
                if (i >= 101 && i <= 102) {
                    s = strtok(line, " ");
                    int k = 0;
                    while (s != NULL) {
                        k++;
                        if (m < 2) {
                            if (k == 1) {
                                if (atoi(s) == 0) {
                                    players[1].hand[m].suit = HEARTS;
                                } if (atoi(s) == 1) {
                                    players[1].hand[m].suit = DIAMONDS;
                                } if (atoi(s) == 2) {
                                    players[1].hand[m].suit = CLUBS;
                                } else if (atoi(s) == 3) {
                                    players[1].hand[m].suit = SPADES;
                                }
                            } else if (k == 2) {
                                players[1].hand[m].rank = atoi(s);
                            }
                        }
                        s = strtok(NULL, " ");
                    }
                    m++;
                }
                if (i == 103) {
                    players[1].isSmallBlind = atoi(line);
                }
                if (i == 104) {
                    players[1].isBigBlind = atoi(line);
                }
            }

            // Load player 3
            if (num_player >= 3) {
                if (i >= 106 && i <= 112) {
                    m = 0;
                    if (i == 106) {
                        players[2].money = atoi(line);
                    }
                    if (i == 107) {
                        players[2].bet = atoi(line);
                    }
                    if (i == 108) {
                        switch (atoi(line)) {
                            case 1: players[2].state = Called;
                                break;
                            case 2: players[2].state = Raised;
                                break;
                            case 3: players[2].state = Checked;
                                break;
                            case 4: players[2].state = Bets;
                                break;
                            case 5: players[2].state = Allins;
                                break;
                            case 6: players[2].state = Folded;
                                break;
                            case 7: players[2].state = SB;
                                break;
                            case 8: players[2].state = BB;
                                break;
                            default: players[2].state = None;
                        }
                    }
                    if (i >= 109 && i <= 110) {
                        s = strtok(line, " ");
                        int k = 0;
                        while (s != NULL) {
                            k++;
                            if (m < 2) {
                                if (k == 1) {
                                    if (atoi(s) == 0) {
                                        players[2].hand[m].suit = HEARTS;
                                    } if (atoi(s) == 1) {
                                        players[2].hand[m].suit = DIAMONDS;
                                    } if (atoi(s) == 2) {
                                        players[2].hand[m].suit = CLUBS;
                                    } else if (atoi(s) == 3) {
                                        players[2].hand[m].suit = SPADES;
                                    }
                                } else if (k == 2) {
                                    players[2].hand[m].rank = atoi(s);
                                }
                            }
                            s = strtok(NULL, " ");
                        }
                        m++;
                    }
                    if (i == 111) {
                        players[2].isSmallBlind = atoi(line);
                    }
                    if (i == 112) {
                        players[2].isBigBlind = atoi(line);
                    }
                }
            }

            // Load player 4
            if (num_player >= 4) {
                if (i >= 114 && i <= 120) {
                    m = 0;
                    if (i == 114) {
                        players[3].money = atoi(line);
                    }
                    if (i == 115) {
                        players[3].bet = atoi(line);
                    }
                    if (i == 116) {
                        switch (atoi(line)) {
                            case 1: players[3].state = Called;
                                break;
                            case 2: players[3].state = Raised;
                                break;
                            case 3: players[3].state = Checked;
                                break;
                            case 4: players[3].state = Bets;
                                break;
                            case 5: players[3].state = Allins;
                                break;
                            case 6: players[3].state = Folded;
                                break;
                            case 7: players[3].state = SB;
                                break;
                            case 8: players[3].state = BB;
                                break;
                            default: players[3].state = None;
                        }
                    }
                    if (i >= 117 && i <= 118) {
                        s = strtok(line, " ");
                        int k = 0;
                        while (s != NULL) {
                            k++;
                            if (m < 2) {
                                if (k == 1) {
                                    if (atoi(s) == 0) {
                                        players[3].hand[m].suit = HEARTS;
                                    } if (atoi(s) == 1) {
                                        players[3].hand[m].suit = DIAMONDS;
                                    } if (atoi(s) == 2) {
                                        players[3].hand[m].suit = CLUBS;
                                    } else if (atoi(s) == 3) {
                                        players[3].hand[m].suit = SPADES;
                                    }
                                } else if (k == 2) {
                                    players[3].hand[m].rank = atoi(s);
                                }
                            }
                            s = strtok(NULL, " ");
                        }
                        m++;
                    }
                    if (i == 119) {
                        players[3].isSmallBlind = atoi(line);
                    }
                    if (i == 120) {
                        players[3].isBigBlind = atoi(line);
                    }
                }
            }

            // Load player 5
            if (num_player == 5) {
                if (i >= 122 && i <= 128) {
                    m = 0;
                    if (i == 122) {
                        players[4].money = atoi(line);
                    }
                    if (i == 123) {
                        players[4].bet = atoi(line);
                    }
                    if (i == 124) {
                        switch (atoi(line)) {
                            case 1: players[4].state = Called;
                                break;
                            case 2: players[4].state = Raised;
                                break;
                            case 3: players[4].state = Checked;
                                break;
                            case 4: players[4].state = Bets;
                                break;
                            case 5: players[4].state = Allins;
                                break;
                            case 6: players[4].state = Folded;
                                break;
                            case 7: players[4].state = SB;
                                break;
                            case 8: players[4].state = BB;
                                break;
                            default: players[4].state = None;
                        }
                    }
                    if (i >= 125 && i <= 126) {
                        s = strtok(line, " ");
                        int k = 0;
                        while (s != NULL) {
                            k++;
                            if (m < 2) {
                                if (k == 1) {
                                    if (atoi(s) == 0) {
                                        players[4].hand[m].suit = HEARTS;
                                    } if (atoi(s) == 1) {
                                        players[4].hand[m].suit = DIAMONDS;
                                    } if (atoi(s) == 2) {
                                        players[4].hand[m].suit = CLUBS;
                                    } else if (atoi(s) == 3) {
                                        players[4].hand[m].suit = SPADES;
                                    }
                                } else if (k == 2) {
                                    players[4].hand[m].rank = atoi(s);
                                }
                            }
                            s = strtok(NULL, " ");
                        }
                        m++;
                    }
                    if (i == 127) {
                        players[4].isSmallBlind = atoi(line);
                    }
                    if (i == 128) {
                        players[4].isBigBlind = atoi(line);
                    }
                }
            } else {
                printf("Shit.");
            }
        }
    } else {
        printf("Can't open file\n");
    }
    fclose(save);
}

int roundPoker(Player *players, Table *table, Deck *deck, int num_player, int roundIdx, int countActivePlayer) {
    int playerIdx  = 0, countCheck = 0, countAllin = 0, countCall = 0, end_round = 0, is_1st_bet = 0, count = 0;
    State lastState = None;

    if (roundIdx > 0) {
        dealSharedCards(table, deck, roundIdx);
        table->last_bet = 0;
    }

    if (roundIdx == 0) {
        for (int b = 0; b < num_player; b++) {
            if (players[b].state != BB && players[b].state != SB) {
                count++;
            }
        }
        if (count == 0 && players[playerIdx].state == BB) {
            if (num_player == 2) {
                is_1st_bet = 1;
            } else {
                is_1st_bet = 0;
            }
        } else {
            lastState = BB;
            is_1st_bet = 1;
        }
        if (is_1st_bet) {
            for (int i = 0; i < num_player; i++) {
                if (players[i].state == BB || players[playerIdx].state == SB) {
                    if (players[i].state == BB) {
                        playerIdx = i + 1;
                        if (playerIdx >= num_player) {
                            playerIdx = 0;
                        }
                        table->last_bet = players[i].bet;
                    }
                    continue;
                }
            }
            table->highest_bet = table->ante * 2;
        }
    } else {
        for (int e = 0; e < num_player; e++) {
            if (players[e].isBigBlind && players[e].status == 1) {
                playerIdx = e;
            } else if (players[e].isSmallBlind && players[e].status == 1) {
                playerIdx = e;
            }
        }
    }
    for (int f = 0; f < num_player; f++) {
        if (players[f].state == Allins) {
            countAllin++;
        }
    }
    if (countAllin == countActivePlayer && is_1st_bet == 0) {
        return countActivePlayer;
    }
    while (!end_round) {
        if (playerIdx >= num_player) {
            playerIdx = 0;
        }
        if (countAllin == countActivePlayer - 1 && players[playerIdx].bet == table->highest_bet) {
            break;
        }
        if (players[playerIdx].state != Folded && players[playerIdx].state != Allins
            && players[playerIdx].money > 0 && players[playerIdx].status == 1) {
            printf("State: %i\n", players[playerIdx].state);
            for (int c = 0; c < num_player; c++) {
                displayPlayerInfo(players[c]);
                printf("\n");
            }
            if (players[playerIdx].state == BB) {
                is_1st_bet = 0;
            }
            displayTableInfo(*table);
            turn(&players[playerIdx], table, roundIdx, playerIdx);
            if (is_1st_bet) {
                if (players[playerIdx].isBigBlind && players[playerIdx].state == Checked) {
                    end_round = 1;
                }
            } else {
                if (players[playerIdx].state == Checked) {
                    countCheck++;
                }
            }
            if (players[playerIdx].state == Allins) {
                countAllin++;
            }
            if (players[playerIdx].state == Called) {
                if (players[playerIdx].money == 0) {
                    players[playerIdx].state = Allins;
                    countAllin++;
                } else {
                    countCall++;
                }
            }
            if (players[playerIdx].state == Folded) {
                countActivePlayer--;
            }
            if (players[playerIdx].state == Raised || players[playerIdx].state == Bets) {
                if (players[playerIdx].money == 0) {
                    players[playerIdx].state = Allins;
                    countAllin++;
                }
                countCall = 0;
            }
            if (countActivePlayer == 1 || countAllin == countActivePlayer || countCheck == countActivePlayer ||
                (countCall == countActivePlayer - 1 && !is_1st_bet) ||
                (players[playerIdx].isBigBlind && lastState == BB && players[playerIdx].state == Folded) ||
                (countAllin == countActivePlayer - 1 && (players[playerIdx].state == Called || players[playerIdx].state == Checked))) {
                lastState = None;
                end_round = 1;
            }
        }
        printf("PlayerIdx: %i\n", playerIdx);
        playerIdx++;
    }
    save(players, table, deck, num_player, roundIdx, playerIdx, countCheck, countAllin, countCall, is_1st_bet);
    printf("Active Player: %i\n", countActivePlayer);
    return countActivePlayer;
}

void checkWinner(Player * players, int num_player) {
    int winner_idx[num_player]; //create an int array which stores idx of winner
    int idx; //idx of winner_idx array

    for (int j = 0; j < 5; j++) {
        //initialize value of winner_idx array
        for (int i = 0; i < num_player; i++) {
            winner_idx[i] = -1;
        }
        idx = 0;

        //create an array to store the j card of each players
        int temp_array[num_player];
        //add rank of j card to the array, if the player is a loser, leave 0
        for (int z = 0; z < num_player; z++) {
            if (players[z].isWinner != 1) {
                temp_array[z] = 0;
            }
            temp_array[z] = players[z].max_hand[j].rank;
        }

        //check if j card is Ace
        int countAce = 0;
        for (int z = 0; z < num_player; z++) {
            if (temp_array[z] == 1) {
                countAce++;
                winner_idx[idx] = z; //add the player_idx to winner_idx
                idx++;
            }
        }

        //if only 1 player has Ace, the rest will be losers
        if (countAce == 1) {
            for (int z = 0; z < num_player; z++) {
                if (z != winner_idx[0]) {
                    players[z].isWinner = 0;
                }
            }
            return; //stop the checking
        }

            //if more than 2 players have Ace, the one who does not have Ace will be a loser
        else if (countAce > 1) {
            for (int z = 0; z < num_player; z++) {
                for (int k = 0; k < idx; k++) {
                    if (z == k) {
                        players[z].isWinner = 1;
                        break; //move to another player
                    } else {
                        players[z].isWinner = 0;
                    }
                }
            }
            continue; //move to another j card
        }

        //if countAce = 0
        int max_rank = temp_array[0];
        int countMax = 0;
        for (int i = 0; i < num_player; i++) {
            if (temp_array[i] > max_rank) {
                idx = 0;
                max_rank = temp_array[i];
                winner_idx[idx] = i;
                idx++;
                countMax = 1;
            } else if (temp_array[i] == max_rank) {
                winner_idx[idx] = i;
                idx++;
                countMax++;
            }
        }

        //if only 1 player has the highest j card, the rest will be losers
        if (countMax == 1) {
            for (int z = 0; z < num_player; z++) {
                if (z != winner_idx[0]) {
                    players[z].isWinner = 0;
                }
            }
            return; //stop the checking
        }

            //if more than 2 players have the highest j card rank, the one who does not have Ace will be a loser
        else if (countMax > 1) {
            for (int z = 0; z < num_player; z++) {
                for (int k = 0; k < idx; k++) {
                    if (z == k) {
                        players[z].isWinner = 1;
                        break; //move to another player
                    } else {
                        players[z].isWinner = 0;
                    }
                }
            }
            continue; //move to another j card
        }
        //countMax should never be 0 at the end
        //because before max_rank changes, the statement: temp_array[0] == max_rank is always true

        //at the end, there can be more than 1 players are winners
    }
}

void testHand(Hand *hands, Player * players, int num_player) {
    sortHands(hands, num_player);
    for (int i = 0; i < num_player; i++) {
        if (players[i].status == 0) {
            continue;
        }
        printf("%s: ", players[i].name);
        for (int j = 0; j < 7; j++) {
            printf("%s %i; ", getSuit(hands[i].card[j].suit), hands[i].card[j].rank);
        }
        if (isRoyalFlush(hands[i], &players[i])) {
            printf("Player %i has royal straight flush.\n", i + 1);
        } else if (isStraightFlush(hands[i], &players[i])) {
            printf("Player %i has straight flush.\n", i + 1);
        } else if (is4OfAKind(hands[i], &players[i])) {
            printf("Player %i has four of a kind.\n", i + 1);
        } else if (isFullHouse(hands[i], &players[i])) {
            printf("Player %i has a fullhouse.\n", i + 1);
        } else if (isFlush(hands[i], &players[i])) {
            printf("Player %i has a flush.\n", i + 1);
        } else if (isStraight(hands[i], &players[i])) {
            printf("Player %i has a straight.\n", i + 1);
        } else if (is3OfAKind(hands[i], &players[i])) {
            printf("Player %i has three of a kind.\n", i + 1);
        } else if (is2Pair(hands[i], &players[i])) {
            printf("Player %i has double pair.\n", i + 1);
        } else if (isPair(hands[i], &players[i])) {
            printf("Player %i has a pair.\n", i + 1);
        } else {
            isHighCard(hands[i], &players[i]);
            printf("Player %i has high card.\n", i + 1);
        }
        printf("Player %i max hand: ", i + 1);
        for (int j = 0; j < 5; j++) {
            printf("%s %i; ", getSuit(players[i].max_hand[j].suit), players[i].max_hand[j].rank);
        }
        printf("\n");
    }


    int countWinners;
    for (Rank rank = RoyalFlush; rank >= HighCard; rank--) {
        countWinners = 0;

        //if the player's rank == the checking rank, add his idx to winner_idx array
        for (int i = 0; i < num_player; i++) {
            if (players[i].rank == rank && players[i].status == 1) {
                players[i].isWinner = 1;
                countWinners++;
            }
        }

        //if there is only 1 winner, stop the checking
        if (countWinners == 1) {
            break; //stop the checking
        }
            //if there is more than 2 winners, check them
        else if (countWinners >= 2) {
            //create a temporary pointer
            Player * temp = malloc(sizeof(Player) * countWinners);
            //copy the value of temporary winners to the addresses where temp pointer points to
            int idx = 0;
            for (int i = 0; i < num_player; i++) {
                if (players[i].isWinner) {
                    temp[idx] = players[i];
                    idx++;
                }
            }

            //check who is the winner
            checkWinner(temp, countWinners);
            //copy back the value of the pointer to the appropriate player
            for (int i = 0; i < num_player; i++) {
                for (int x = 0; x < idx; x++) {
                    if (strcmp(players[i].name, temp[x].name) == 0) {
                        players[i] = temp[x];
                    }
                }
            }

            free(temp);
            break; //stop the checking
        }
    }
}

void award(Player * players, Table * table, int num_player) {
    int winnersMoney = 0;
    int count_fold = 0;

    for (int i = 0; i < num_player; i++) {
        if (players[i].isWinner) {
            winnersMoney += players[i].bet;
        }
        if (players[i].status == 0) {
            count_fold++;
        }
    }
    //split pot by ratio; if there is only one active player, he takes all the chips.
    for (int i = 0; i < num_player; i++) {
        if (players[i].isWinner) {
            if (count_fold == num_player - 1) {
                players[i].money += table->pot_money;
            } else {
                players[i].money += players[i].bet / winnersMoney * table->pot_money;
            }
            printf("%s is the winner!!!", players[i].name);
        }
    }
    table->pot_money = 0;
}

void reset (Player * players, Table * table, int num_player, Deck * deck) {
    for (int i = 0; i < num_player; i++) {
        players[i].bet = 0;
        players[i].isWinner = 0;
        players[i].option = Call;
        players[i].state = None;
        players[i].status = 1;
        for (int j = 0; j < 5; j++) {
            if (j < 2) {
                players->hand[j].rank = 0;
            }
            players[i].max_hand[j].rank = 0;
        }
    }
    deck->card_index = 0;
    table->pot_money = 0;
    table->card_idx = 0;
    for (int i = 0; i < 5; i++) {
        table->card[i].rank = 0;
    }
    table->highest_bet = 0;
    table->last_bet = 0;
}

int game (Player * players, Table * table, Deck * deck, int num_player, int gameIdx, int nextSB) {
    int prevPlayer = nextSB - 1, nextBB = nextSB + 1;
    int countActivePlayer = num_player;
    if (players[nextSB].money == 0) {
        for (int i = 0; i < num_player; i++) {
            nextSB++;
            if (nextSB == num_player){
                nextSB = 0;
            }
            if (players[nextSB].money > 0) {
                nextBB = nextSB + 1;
                break;
            }
        }
    }

    if (players[nextBB].money == 0) {
        for (int i = 0; i < num_player; i++) {
            nextBB++;
            if (nextBB == num_player){
                nextBB = 0;
            }
            if (players[nextBB].money > 0) {
                break;
            }
        }
    }

    if (nextSB == num_player - 1) {
        nextBB = 0;
    }
    if (nextSB >= num_player) {
        nextSB = 0;
        nextBB = 1;
    }

    for (int i = 0; i < num_player; i++) {
        if (i != nextSB) {
            players[i].isSmallBlind = 0;
        }
        if (players[i].money == 0) {
            players[i].isBigBlind = 0;
            players[i].isSmallBlind = 0;
        }
    }

    table->ante = 250;
    reset(players, table, num_player, deck);
    players[nextSB].isSmallBlind = 1;
    players[nextSB].state = SB;
    if (players[nextSB].money <= table->ante) {
        players[nextSB].bet = players[nextSB].money;
    } else {
        players[nextSB].bet = table->ante;
    }
    players[nextSB].money = players[nextSB].money - players[nextSB].bet;
    players[nextBB].isBigBlind = 1;
    players[nextBB].state = BB;
    if (players[nextBB].money <= table->ante * 2) {
        players[nextBB].bet = players[nextBB].money;
    } else {
        players[nextBB].bet = table->ante * 2;
    }
    players[nextBB].money = players[nextBB].money - players[nextBB].bet;
    players[prevPlayer].isSmallBlind = 0;
    players[nextSB].isBigBlind = 0;
    table->pot_money = players[nextSB].bet + players[nextBB].bet;
    nextSB++;
    if (nextSB == num_player) {
        nextSB = 0;
    }

    dealStartingHand(players, deck, num_player);
    for (int i = 0; i < num_player; i++) {
        if (players[i].money <= 0) {
            players[i].status = 0;
            countActivePlayer--;
        }
    }

    int countFold = 0;
    int roundIdx;
    for (roundIdx = 0; roundIdx < 4; roundIdx++) {
        countActivePlayer = roundPoker(players, table, deck, num_player, roundIdx, countActivePlayer);
        printf("-------End round--------\n");
        countFold = 0;
        for (int i = 0; i < num_player; i++) {
            if (players[i].status == 0) {
                countFold++;
            }
        }
        if (countFold == num_player - 1) {
            //TODO: erase printf after finish the project
            printf("Round Idx: %i\n", roundIdx);
            break;
        }
    }
    if (countFold == num_player - 1 && roundIdx < 4 ) {
        for (int i = 0; i < num_player; i++) {
            if (players[i].status == 1) {
                players[i].isWinner = 1;
                break;
            }
        }
    } else {
        Hand *hands = createHands(players, table, num_player);
        testHand(hands, players, num_player);
        free(hands);
    }
    award(players, table, num_player);
    printf("Next Blind: %i\n", nextSB);
    return nextSB;
}

void mainMenu(){
    printf("----------POKER----------\n1. Start game\n2. Options\n3. Exit\nYour option: ");
}

int main() {
//    int opt = 0;
//    int opt1 = 0;
//    int opt2 = 0;
//    int endProgram = 0;
//    while (!endProgram) {
//        int nextBlind = 0;
//        mainMenu();
//        scanf("%d", &opt);
//        if (opt == 1) {
//            // Create table
//            Table *table = createTable();
//
//            // Create deck
//            Deck *deck;
//            deck = newDeck();
//            int size = 52;
//
//            // Test new deck
//            for (int m = 0; m < size; m++) {
//                printf("%s %i; ", getSuit(deck->cards[m].suit), deck->cards[m].rank);
//            }
//            printf("\n");
//
//            // Create players
//            int num_player = 5;
//            Player *players = createPlayers(num_player);
//            for (int gameIdx = 0; ; gameIdx++) {
//                int remain = num_player;
//                printf("Gameidx: %i\n", gameIdx);
//                for (int m = 0; m < num_player; m++) {
//                    if (players[m].money <= 0) {
//                        remain--;
//                    }
//                }
//                printf("remain: %i\n", remain);
//                if (remain == 1) {
//                    printf("No player left.\n");
//                    break;
//                }
//                // Shuffle the deck
//                shuffleDeck(deck, size);
//
//                // Test shuffle
//                for (int m = 0; m < size; m++) {
//                    printf("%s %i; ", getSuit(deck->cards[m].suit), deck->cards[m].rank);
//                }
//                printf("\n");
//                nextBlind = game(players, table, deck, num_player, gameIdx, nextBlind);
//                printf("Continue? (0/1): ");
//                scanf("%d", &opt1);
//                if (opt1 == 0) {
//                    break;
//                }
//            }
//            // Free everything
//            for (int i = 0; i < num_player; i++) {
//                free(players[i].max_hand);
//            }
//            free(players);
//            free(deck);
//            free(table);
//        } else if (opt == 2) {
//            printf("Options\n");
//        } else if (opt == 3) {
//            endProgram = 1;
//        }
//    }


    // Create table
    Table *table = createTable();

    // Create deck
    Deck *deck;
    deck = newDeck();

    int num_player = loadNumPlayer();

    // Create players
    Player *players = createPlayers(num_player);

    int roundIdx;
    loadRoundInfo(-1, -1, -1, -1);

    load(table, deck, players, &roundIdx);

    for (int m = 0; m < 52; m++) {
        printf("%s %i; ", getSuit(deck->cards[m].suit), deck->cards[m].rank);
    }
    printf("\n");
    displayTableInfo(*table);

    for (int n = 0; n < num_player; n++) {
        displayPlayerInfo(players[n]);
        printf("\n");
    }


//    printf("Money: %i\n", players[0].money);
//    printf("Bet: %i\n", players[0].bet);
//    printf("State: %i\n", players[0].state);
//    printf("Hand: ");
//    for (int n = 0; n < 2; n++) {
//        printf("%s %i; ", getSuit(players[0].hand[n].suit), players[0].hand[n].rank);
//    }
//    printf("\n");
//    printf("Is SB: %i\n", players[0].isSmallBlind);
//    printf("Is BB: %i\n", players[0].isBigBlind);

    return 0;
}
