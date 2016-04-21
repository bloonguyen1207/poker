//
// Created by bloo on 12/04/2016.
//

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

enum suit {HEARTS, DIAMONDS, CLUBS, SPADES};
typedef enum suit Suit;

enum rank {HighCard, OnePair, TwoPairs, Three, Four, Straight, Flush, FullHouse, StraightFlush};
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
    Card hand[2];
    int money;
    Card max_hand[5];
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
        Player* player = malloc(sizeof(Player) * no_player);
        for (int i = 0; i < no_player; i++) {
            strcpy(player[i].name, "Player 0");
            player[i].name[7] += i + 1;
            player[i].money = 5000;
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

int isHighCard(Hand hand) {
    return 0;
}

int seachHandRank(Hand hand, int target) {
    for (int i = 0; i < 7; i++) {
        if (hand.card[i].rank == target) {
            return 1;
        }
    }
    return 0;
}

int isPair(Hand hand){
    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 7; j++) {
            if(hand.card[i].rank == hand.card[j].rank) {
                return 1;
            }
        }
    }
    return 0;
}

int is2Pair(Hand hand) {
    int count = 0;
    int exist = 0;
    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 7; j++) {
            if(hand.card[i].rank == hand.card[j].rank && hand.card[i].rank != exist) {
                exist = hand.card[i].rank;
                count++;
            }
            if (count >= 2) {
                return 1;
            }
        }
    }
    exist = 0;
    count = 0;
    return 0;
}

int is3OfAKind(Hand hand) {
    int exist = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = i + 1; j < 6; j++) {
            for (int k = j + 1; k < 7; k++) {
                if (hand.card[i].rank == hand.card[j].rank &&
                    hand.card[i].rank == hand.card[k].rank &&
                    hand.card[i].rank != exist) {
                    exist = hand.card[i].rank;
                    return 1;
                }
            }
        }
    }
    exist = 0;
    return 0;
}

int isStraight(Hand hand) {
    return 0;
}

//the hand must be sort before checking rank
int isFlush(Hand* hand, Player* player, int num_player) {
    for (int i = 0; i < num_player; i++) {
        for (Suit suit = HEARTS; suit <= SPADES; suit++) {
            int count = 0; //count number of cards that have the same suit
            for (int j = 0; j < 7; j++) {
                if (hand[i].card[j].suit == suit) {
                    player[i].max_hand[count] = hand[i].card[j]; //add card to max_hand
                    count++;
                    if (count == 5) { //hand has been sorted --> the max_hand contains 5 cards that has the highest rank.
                        player[i].rank = Flush;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int is4OfAKind(Hand hand) {
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 5; j++) {
            for (int k = j + 1; k < 6; k++) {
                for (int l = k + 1; l < 7; l++) {
                    if (hand.card[i].rank == hand.card[j].rank &&
                        hand.card[i].rank == hand.card[k].rank &&
                        hand.card[i].rank == hand.card[l].rank) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

int isFullHouse(Hand hand) {
    return 0;
}

int isStraightFlush(Hand hand) {
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
        if (is4OfAKind(hands[i])) {
            printf("Player %i has four of a kind.", i + 1);
        } else if (is3OfAKind(hands[i])) {
            printf("Player %i has three of a kind.", i + 1);
        } else if (is2Pair(hands[i])) {
            printf("Player %i has double pair.", i + 1);
        } else if (isPair(hands[i])) {
            printf("Player %i has a pair.", i + 1);
        }
        printf("\n");
    }

    // Free everything
    free(player);
    free(deck);
    free(table);
    free(hands);
    return 0;
}

