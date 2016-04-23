//
// Created by bloo on 12/04/2016.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

enum suit {HEARTS, DIAMONDS, CLUBS, SPADES};
typedef enum suit Suit;

enum rank {HighCard, OnePair, TwoPairs, Three, Four, Straight, Flush, FullHouse, StraightFlush, RoyalStraightFlush};
typedef enum rank Rank;

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
};
typedef struct player Player;

struct table {
    int pot_money;
    Card card[5];
    int card_idx;
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

Player* createPlayers(int no_player) {
    if (no_player >= 2 && no_player <= 10) {
        Player *player = malloc(sizeof(Player) * no_player);
        for (int i = 0; i < no_player; i++) {
            strcpy(player[i].name, "Player 0");
            player[i].status = 1;
            player[i].name[7] += i + 1;
            player[i].money = 5000;
            player[i].max_hand = malloc(sizeof(Card) * 5);
        }
        return player;
    }
    return NULL;
}

Table * createTable() {
    Table * table = malloc(sizeof(Table));
    table->card_idx = 0;
    return table;
}

void dealStartingHand(int no_player, Player* player, Deck* deck) {
    for (int j = 0; j < 2; j++) {
        for (int i = 0; i < no_player; i++) {
            player[i].hand[j] = deck->cards[deck->card_index];
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

Hand* createHand(Player *players, Table *table, int num_player) {
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

void swapCards(Hand* hand, int player, int fstIdx, int secIdx) {
    Card temp = hand[player].card[fstIdx];
    hand[player].card[fstIdx] = hand[player].card[secIdx];
    hand[player].card[secIdx] = temp;
}

void sortHand(Hand* hand, int num_player) {
    for (int i = 0; i < num_player; i++) {
        int max_rank;
        for (int j = 0; j < 7; j++) {
            max_rank = hand[i].card[j].rank;
            for (int k = j+1; k < 7; k++) {
                if (hand[i].card[k].rank > max_rank) {
                    max_rank = hand[i].card[k].rank;
                    swapCards(hand, i, j, k);
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

Card isHighestCard(Hand hand, Player player) {
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
        int temp = hand.card[j].rank;
        player->max_hand[check] = searchCard(hand, temp);
        check++;
        if (temp == 13) {
            for (int k = 12; k >= 10; k--) {
                if (searchHandRank(hand, k)) {
                    player->max_hand[check] = searchCard(hand, k);
                    check++;
                };
            }
            if (searchHandRank(hand, 1)) {
                player->max_hand[check] = searchCard(hand, 1);
                check++;
            };
            if (check == 5) {
                player->rank = Straight;
                return 1;
            }
        }

        if (check > 1) {
            check = 1;
        }
        for (int k = 1; k < 5; k++) {
            if (temp - k > 0 && searchHandRank(hand, temp - k)) {
                player->max_hand[check] = searchCard(hand, temp - k);
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
            if (hand.card[j].suit == suit) {
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
            if (hand.card[j].suit == suit) {
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

int isRoyalStraightFlush(Hand hand, Player* player) {
    for (Suit suit = HEARTS; suit <= SPADES; suit++) {
        Hand* temp = malloc(sizeof(Hand));
        int count = 0; //count number of cards that have the same suit
        int check = 0;
        for (int j = 0; j < 7; j++) {
            if (hand.card[j].suit == suit) {
                temp->card[count] = hand.card[j];
                count++;
            }
        }
        if (count >= 5) {
            for (int i = 13; i >= 10; i--) {
                if (searchHandRank(*temp, i)) {
                    player->max_hand[check] = searchCard(*temp, i); //add card to max_hand
                    check++;
                };
            }
            if (searchHandRank(*temp, 1)) {
                player->max_hand[check] = searchCard(*temp, 1); //add card to max_hand
                check++;
            };
            if (check == 5) {
                free(temp);
                player->rank = RoyalStraightFlush;
                return 1;
            }
        }
        free(temp);
    }
    return 0;
}

int main() {
    // Create table
    Table *table = createTable();

    // Create deck
    Deck * deck;
    deck = newDeck();
    int size = 52;

    // Test new deck
    for (int m = 0; m < size; m++) {
        printf("%s %i; ", getSuit(deck->cards[m].suit), deck->cards[m].rank);
    }
    printf("\n");

    // Shuffle the deck
    shuffleDeck(deck, size);

    // Test shuffle
    for (int m = 0; m < size; m++) {
        printf("%s %i; ", getSuit(deck->cards[m].suit), deck->cards[m].rank);
    }
    printf("\n");

    // Create players
    int no_player = 2;
    Player* player = createPlayers(no_player);

    // Deal hole cards for players
    dealStartingHand(no_player, player, deck);

    // Deal shared cards
    dealSharedCards(table, deck, 1);
    dealSharedCards(table, deck, 2);
    dealSharedCards(table, deck, 3);

    // Test player starting hand
    for (int i = 0; i < no_player; i++) {
        printf("%s: ", player[i].name);
        for (int j = 0; j < 2; j++) {
            printf("%s %i; ", getSuit(player[i].hand[j].suit), player[i].hand[j].rank);
        }
        printf("\n");
    }

    // Test shared cards
    printf("Shared Cards: ");
    for (int i = 0; i < 5; i++) {
        printf("%s %i; ", getSuit(table->card[i].suit), table->card[i].rank);
    }
    printf("\n");

    // Test hands
    Hand *hands = createHand(player, table, no_player);
    sortHand(hands, no_player);
    for (int i = 0; i < no_player; i++) {
        printf("%s: ", player[i].name);
        for (int j = 0; j < 7; j++) {
            printf("%s %i; ", getSuit(hands[i].card[j].suit), hands[i].card[j].rank);
        }
        if (isRoyalStraightFlush(hands[i], &player[i])) {
            printf("Player %i has royal straight flush.\n", i + 1);
        } else if (isStraightFlush(hands[i], &player[i])) {
            printf("Player %i has straight flush.\n", i + 1);
        } else if (is4OfAKind(hands[i], &player[i])) {
            printf("Player %i has four of a kind.\n", i + 1);
        } else if (isFullHouse(hands[i], &player[i])) {
            printf("Player %i has a fullhouse.\n", i + 1);
        } else if (isFlush(hands[i], &player[i])) {
            printf("Player %i has a flush.\n", i + 1);
        } else if (isStraight(hands[i], &player[i])) {
            printf("Player %i has a straight.\n", i + 1);
        } else if (is3OfAKind(hands[i], &player[i])) {
            printf("Player %i has three of a kind.\n", i + 1);
        } else if (is2Pair(hands[i], &player[i])) {
            printf("Player %i has double pair.\n", i + 1);
        } else if (isPair(hands[i], &player[i])) {
            printf("Player %i has a pair.\n", i + 1);
        } else {
            isHighCard(hands[i], &player[i]);
            printf("\nPlayer %i highest card: %s %i -- ", i + 1, getSuit(isHighestCard(hands[i], player[i]).suit), isHighestCard(hands[i], player[i]).rank);
        }
        printf("Player %i max hand: ", i + 1);
        for (int j = 0; j < 5; j++) {
            printf("%s %i; ", getSuit(player[i].max_hand[j].suit), player[i].max_hand[j].rank);
        }
        printf("\n");
    }
//    Player *test_player = malloc(sizeof(Player));
//    test_player->max_hand = malloc(sizeof(Card) * 5);
//    Hand *test = malloc(sizeof(Hand));
//    test->card[0].rank = 13; test->card[0].suit = HEARTS;
//    test->card[1].rank = 13; test->card[1].suit = HEARTS;
//    test->card[2].rank = 13; test->card[2].suit = HEARTS;
//    test->card[3].rank = 12; test->card[3].suit = HEARTS;
//    test->card[4].rank = 10; test->card[4].suit = HEARTS;
//    test->card[5].rank = 10; test->card[5].suit = DIAMONDS;
//    test->card[6].rank = 2; test->card[6].suit = HEARTS;
//    for (int j = 0; j < 7; j++) {
//        printf("%s %i; ", getSuit(test->card[j].suit), test->card[j].rank);
//    }
//    if (isFullHouse(*test, test_player)) {
//        printf("True");
//    } else {printf("False");}
//    printf("\n%d\n", test_player->rank);
//    for (int j = 0; j < 5; j++) {
//        printf("%s %i; ", getSuit(test_player->max_hand[j].suit), test_player->max_hand[j].rank);
//    }

    // Free everything
    free(player);
    free(deck);
    free(table);
    free(hands);

//    free(test);
//    free(test_player->max_hand);
//    free(test_player);
    return 0;
}

