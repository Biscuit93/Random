#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_RANKS 13
#define NUM_SUITS 4
#define NUM_HANDS 10
#define DECK_SIZE NUM_RANKS * NUM_SUITS
#define HAND_SIZE 5
#define PILE_SIZE HAND_SIZE
#define EMPTY     0xFF

char *card        ( int index );
void printCards   ( int *loc, int num );
void initStack    ( int *loc, int num, int empty );
void shuffle      ( int *loc, int num );
void swap         ( int *a, int *b );
void sort         ( int *loc, int size, int byRank );
void copy         ( int *old, int *new, int size );
int  moveCard     ( int *from, int *to, int fromSize, int toSize );
int  moveCards    ( int num, int *from, int *to, int fromSize, int toSize );
int  rankHand     ( int *hand );
int  isStraight   ( int *hand, int *high );
int  isFlush      ( int *hand );
void countMatches ( int *hand, int *four, int *three, int *pair1, int *pair2 );
int  isEmpty      ( int *loc, int size );
int  isFull       ( int *loc, int size );
void printRound   ( );
int  promptYesNo  ( char *prompt );
int  promptNumber ( char *prompt, int lower, int upper );
int  *promptSwap  ( );
void exchange     ( int *hand, int *deck, int *pile );



int main ( int argc, char **argv )
{
	static const int PAYOUT[ NUM_HANDS ] = { 0, 1, 2, 3, 4, 6, 9, 25, 50, 800 };
	int credits;
	int wager;
	int prize;
	int rank;

	int deck[ DECK_SIZE + 1 ];
	int pile[ PILE_SIZE + 1 ];
	int hand[ HAND_SIZE + 1 ];

	initStack( deck, DECK_SIZE, 0 );
	initStack( pile, PILE_SIZE, 1 );
	initStack( hand, HAND_SIZE, 1 );

	puts( "\n=== VIDEO POKER ===\n" );

	credits = 100;

	do
	{
		printRound( );

		wager = promptNumber( "Wager how many credits? (0 to quit)", 
		                       0, credits );
		
		if ( wager == 0 )
			break;
		
		credits -= wager;

		shuffle( deck, DECK_SIZE );

		moveCards( HAND_SIZE, deck, hand, DECK_SIZE, HAND_SIZE );

		sort( hand, HAND_SIZE, 1 );
		printCards( hand, HAND_SIZE );

		exchange( hand, deck, pile );

		sort( hand, HAND_SIZE, 1 );
		printCards( hand, HAND_SIZE );

		rank = rankHand( hand );
		prize = wager * PAYOUT[ rank ];
		credits += prize;
		printf( "Credits: $%d ", credits );
		if ( prize )
			printf( "(+$%d)\n", prize );
		else
			puts( "" );

		moveCards( HAND_SIZE, hand, deck, HAND_SIZE, DECK_SIZE );
		moveCards( PILE_SIZE, pile, deck, PILE_SIZE, DECK_SIZE );
	}
	while ( credits > 0 );

	puts( "\n=== GAME OVER ===\n" );

	//printCards( deck, DECK_SIZE );

	return 0;
}

char *card ( int index )
{
	static const char rank[ NUM_RANKS ][ 4 ] = { "A", "2", "3", "4", "5", 
	                                             "6", "7", "8", "9", "10", 
	                                             "J", "Q", "K" };
	static const char suit[ NUM_SUITS ][ 4 ] = { "\u2660", "\u2665", 
	                                             "\u2663", "\u2666" };
	static char result[ 8 ];

	if ( index == EMPTY )
		return "*";
	if ( index >= DECK_SIZE )
		return NULL;

	result[ 0 ] = '\0';
	strcat( result, rank[ index % NUM_RANKS ] );
	strcat( result, suit[ index / NUM_RANKS ] );

	return result;
}

void printCards ( int *loc, int num )
{
	int i = 0;

	puts( "" );

	while ( i < num )
	{
		printf ( "%6s", card( loc[ i ] ) );
		if ( ( i + 1 ) % NUM_RANKS == 0 )
			puts( "" );
		i++;
	}

	puts( "\n" );
}

void initStack ( int *loc, int num, int empty )
{
	for ( int i = 0; i < num; i++ )
		if ( empty )
			loc[ i ] = EMPTY;
		else
			loc[ i ] = i;
	
	if ( empty )
		loc[ num ] = 0;
	else
		loc[ num ] = num;
}

void shuffle ( int *loc, int num )
{
	static int init = 0;
	static time_t t;
	
	if ( !init )
	{
		init = 1;
		srand( ( unsigned ) time( &t ) );
	}

	for ( int i = 0, j; i < num - 2; i++ )
	{
		j = ( rand( ) % ( num - i ) ) + i;
		swap( loc + i , loc + j );
	}
}

void swap ( int *a, int *b )
{
	int temp;

	temp = *b;
	*b   = *a;
	*a   = temp;
}

void sort ( int *loc, int size, int byRank )
{
	int min;

	for ( int i = 0; i < size - 1; i++ )
	{
		min = i;
		for ( int j = i + 1; j < size; j++ )
			if ( byRank )
			{
				if ( loc[ j ] % NUM_RANKS < 
				     loc[ min ] % NUM_RANKS )
					min = j;
			}
			else
			{
				if ( loc[ j ] < loc[ min ] )
					min = j;
			}

		if ( min != i )
			swap( loc + i, loc + min );
	}
}

void copy ( int *old, int *new, int size )
{
	for ( int i = 0; i < size; i++ )
		new[ i ] = old[ i ];
}

int moveCard ( int *from, int *to, int fromSize, int toSize )
{
	if ( isEmpty( from, fromSize ) )
		return 1;

	if ( isFull( to, toSize ) )
		return 2;

	to[ to[ toSize ] ] = from[ from[ fromSize ] - 1 ];
	to[ toSize ]++;
	from[ from[ fromSize ] - 1 ] = EMPTY;
	from[ fromSize ]--;
	
	return 0;
}

int moveCards ( int num, int *from, int *to, int fromSize, int toSize )
{
	int i = 0;

	while ( !isEmpty( from, fromSize ) && !isFull( to, toSize ) 
	        && i++ < num )
		moveCard( from, to, fromSize, toSize );
}

int rankHand ( int *hand )
{
	int four     = EMPTY,
	    three    = EMPTY,
	    pair1    = EMPTY,
	    pair2    = EMPTY,
	    high     = EMPTY,
	    straight = isStraight( hand, &high ),
	    flush    = isFlush( hand );

	countMatches( hand, &four, &three, &pair1, &pair2 );

	// Royal Flush
	if ( straight && flush && high == 0 )
		return 9;
	
	// Straight Flush
	if ( straight && flush )
		return 8;
	
	// Four of a kind
	if ( four != EMPTY )
		return 7;
	
	// Full House
	if ( three != EMPTY && pair1 != EMPTY )
		return 6;
	
	// Flush
	if ( flush )
		return 5;
	
	// Straight
	if ( straight )
		return 4;
	
	// Three of a kind
	if ( three != EMPTY && pair1 == EMPTY )
		return 3;
	
	// Two Pair
	if ( pair1 != EMPTY && pair2 != EMPTY )
		return 2;
	
	// Jacks or better
	if ( pair1 != EMPTY && ( pair1 >= 10 || pair1 == 0 ) )
		return 1;
	
	// Other
	return 0;
}

int isStraight ( int *hand, int *high )
{
	int temp[ HAND_SIZE + 1 ];

	copy( hand, temp, HAND_SIZE );

	for ( int i = 0; i < HAND_SIZE; i++ )
		temp[ i ] %= NUM_RANKS;

	sort( temp, HAND_SIZE, 0 );

	*high = temp[ 4 ];

	return ( ( temp[ 0 ] == ( temp[ 1 ] - 1 ) && 
	           temp[ 1 ] == ( temp[ 2 ] - 1 ) &&
	           temp[ 2 ] == ( temp[ 3 ] - 1 ) &&
	           temp[ 3 ] == ( temp[ 4 ] - 1 ) ) ||
	         ( temp[ 0 ] == 0 &&
		   temp[ 1 ] == 9 &&
		   temp[ 2 ] == 10 &&
		   temp[ 3 ] == 11 &&
		   temp[ 4 ] == 12 ) );
}

int isFlush ( int *hand )
{
	return ( hand[ 0 ] / NUM_RANKS == hand[ 1 ] / NUM_RANKS &&
	         hand[ 1 ] / NUM_RANKS == hand[ 2 ] / NUM_RANKS &&
	         hand[ 2 ] / NUM_RANKS == hand[ 3 ] / NUM_RANKS &&
	         hand[ 3 ] / NUM_RANKS == hand[ 4 ] / NUM_RANKS );
}

void countMatches ( int *hand, int *four, int *three, int *pair1, int *pair2 )
{
	int count;

	*four  = EMPTY;
	*three = EMPTY;
	*pair1 = EMPTY;
	*pair2 = EMPTY;

	for ( int i = 0; i < HAND_SIZE - 1; i++ )
	{
		count = 1;
		for ( int j = i + 1; j < HAND_SIZE; j++ )
		{
			if ( ( hand[ i ] % NUM_RANKS ) == 
			     ( hand[ j ] % NUM_RANKS ) )
				count++;
		}
		switch ( count )
		{
			case 4:
				*four = hand[ i ] % NUM_RANKS;
				return;
			break;
			case 3:
				*three = hand[ i ] % NUM_RANKS;
			break;
			case 2:
				if ( hand[ i ] % NUM_RANKS == 
					*three % NUM_RANKS )
					continue;
				if ( *pair1 == EMPTY )
					*pair1 = hand[ i ] % NUM_RANKS;
				else
					*pair2 = hand[ i ] % NUM_RANKS;
			break;
			default:
				continue;
			break;
		}
	}
}

int isEmpty ( int *loc, int size )
{
	return loc[ size ] == 0;
}

int isFull ( int *loc, int size )
{
	return loc[ size ] == size;
}

void printRound ( )
{
	static int round = 1;

	printf ( "\n=== Round %d ===\n\n", round++ );
}

int promptYesNo ( char *prompt )
{
	char input;

	printf( "%s (y/n) : ", prompt );
	
	while ( 1 )
	{
		input = getchar( );

		if ( input == 'Y' || input == 'y' )
			return 1;
		if ( input == 'N' || input == 'n' )
			return 0;
	}
}

int promptNumber ( char *prompt, int lower, int upper )
{
	int input;

	printf( "%s (%d-%d) : ", prompt, lower, upper );

	while ( 1 )
	{
		scanf( "%d", &input );

		if ( input >= lower && input <= upper )
			return input;
	}
}

int *promptSwap ( )
{
	static int result[ HAND_SIZE ];
	static const char cardinals[ 4 ][ 4 ] = { "1st", "2nd", "3rd", "4th" };
	static const char prompt[ 20 ] = " card to exchange";
	char fullPrompt[ 25 ] = "\0";
	
	int cards = promptNumber( "Exchange how many cards?", 0, 5 );

	for ( int i = 0; i < HAND_SIZE; i++ )
	{
		if ( cards == 0 )
			result[ i ] = EMPTY;
		else if ( cards == HAND_SIZE )
			result[ i ] = i;
		else
		{
			strcat( fullPrompt, cardinals[ i ] );
			strcat( fullPrompt, prompt );
			if ( i < cards )
				result[ i ] = promptNumber( fullPrompt, 0, 5 ) - 1;
			else
				result[ i ] = EMPTY;
			
			fullPrompt[ 0 ] = '\0';
		}
	}

	return result;
}

void exchange ( int *hand, int *deck, int *pile )
{
	static int hold[ HAND_SIZE + 1 ];
	int *discard = promptSwap( ),
	    cardTossed, 
	    count = 0;
	initStack( hold, HAND_SIZE, 1 );

	for ( int i = HAND_SIZE - 1; i >= 0; i-- )
	{
		cardTossed = 0;
		
		for ( int j = 0; j < HAND_SIZE; j++ )
		{
			if ( i == discard[ j ] )
			{
				moveCard( hand, pile, HAND_SIZE, PILE_SIZE );
				cardTossed = 1;
				count++;
				break;
			}
		}

		if ( !cardTossed )
			moveCard( hand, hold, HAND_SIZE, HAND_SIZE );
	}

	moveCards( HAND_SIZE - count, hold, hand, HAND_SIZE, HAND_SIZE );
	moveCards( count, deck, hand, DECK_SIZE, HAND_SIZE );
}

