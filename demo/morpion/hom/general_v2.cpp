// author : S. Hong,Y.Thierry-Mieg
// date : Jan 2009
#include "general_v2.hpp"

#include <iostream>
#include <sstream>
#include <boost/functional/hash.hpp>
#include <vector>



#include "notew.hpp"
#include "nowinner.hpp"
#include "play.hpp"
#include "winner.hpp"



// The player designated plays in a free cll, if any are available.
Hom
PlayAnyFreeCell ()
{
	std::set<GHom> nextAAset;

	for ( size_t i = 0; i < NBCELL; ++i )
	{
		nextAAset.insert ( Play ( i ) );
	}

	return GHom::add ( nextAAset );
}




Hom
generateLineWinner(size_t max, game_status_type player)
{
	std::set<GHom> winAAset;

	/* For each line configuration of the grid game, where i is the line number and j the column number */
	for(size_t i=0;i<max;++i)
	{
		size_t j=0;
		Hom cells = CheckCellWinner ( player, (j + (i*max)) );;
		for(j=1;j<max;++j)
		{
			cells = cells & CheckCellWinner ( player, (j + (i*max)) );
		}
		winAAset.insert (cells);
	}

	return GHom::add ( winAAset );
}


Hom
generateColumnWinner(size_t max, game_status_type player)
{
	std::set<GHom> winAAset;

	/* For each column configuration of the grid game, where j is the line number and i the column */
	for(size_t i=0;i<max;++i)
	{
		size_t j=0;
		Hom cells = CheckCellWinner ( player, (i + (j*max)) );;
		for(j=1;j<max;++j)
		{
			cells = cells & CheckCellWinner ( player, (i + (j*max)) );
		}
		winAAset.insert (cells);
	}

	return GHom::add ( winAAset );
}

Hom
generateCrossWinner(size_t max, game_status_type player)
{
	std::set<GHom> winAAset;


	/* First Cross configuration */
	size_t i=0;
	Hom cell_1 = CheckCellWinner ( player, (i + (i*max)) );
	Hom cell_2 = CheckCellWinner ( player, (((max - 1 - i) + (i*max) ) ) );
	for(i=1;i<max;++i)
	{
		cell_1 = cell_1 & CheckCellWinner ( player, (i + (i*max)) );
		cell_2 = cell_2 & CheckCellWinner ( player, (((max - 1 - i) + (i*max) ) ) );
	}
	winAAset.insert (cell_1);
	winAAset.insert (cell_2);

	return GHom::add ( winAAset );
}

// check if a player has won
// Select any path where the designated player has won : i.e has three aligned cross or circle.
Hom
CheckIsWinner ( game_status_type player )
{

	Hom winners = generateLineWinner(LINE, player) +
	generateColumnWinner(LINE, player) +
	generateCrossWinner(LINE, player);

	return winners;
}


// Strategy marker
const int NOWINNER_STRAT = 0;

Hom CheckNoWinner ()
{
	if ( NOWINNER_STRAT == 0 )
	{
		// NEW !! use a negation : no winner = not ( A wins or B wins )
		Hom res = ! ( CheckIsWinner ( PA ) + CheckIsWinner ( PB ) );
		return res;
	}
	else
	{
		// copy paste from main
		Hom noWinner;
		typedef std::vector<game_status_type> players_array_type;
		players_array_type players;
		players.push_back(EMPTY);
		players.push_back(PA);
		players.push_back(PB);

		for(players_array_type::const_iterator i = players.begin() ; i != players.end() ; ++i)
		{
			if ( *i == PA )
			{
				noWinner = ( CheckCellNoWinner ( *i, 0 ) + CheckCellNoWinner ( *i, 1 ) + CheckCellNoWinner ( *i, 2 ) ) ;
			}
			else
			{
				noWinner = noWinner & ( CheckCellNoWinner ( *i, 0 ) + CheckCellNoWinner ( *i, 1 ) + CheckCellNoWinner ( *i, 2 ) ) ;
			}

			noWinner = noWinner & ( CheckCellNoWinner ( *i, 3 ) + CheckCellNoWinner ( *i, 4 ) + CheckCellNoWinner ( *i, 5 ) ) ;

			noWinner = noWinner & ( CheckCellNoWinner ( *i, 6 ) + CheckCellNoWinner ( *i, 7 ) + CheckCellNoWinner ( *i, 8 ) ) ;

			noWinner = noWinner & ( CheckCellNoWinner ( *i, 0 ) + CheckCellNoWinner ( *i, 3 ) + CheckCellNoWinner ( *i, 6 ) ) ;
			noWinner = noWinner & ( CheckCellNoWinner ( *i, 1 ) + CheckCellNoWinner ( *i, 4 ) + CheckCellNoWinner ( *i, 7 ) ) ;
			noWinner = noWinner & ( CheckCellNoWinner ( *i, 2 ) + CheckCellNoWinner ( *i, 5 ) + CheckCellNoWinner ( *i, 8 ) ) ;

			noWinner = noWinner & ( CheckCellNoWinner ( *i, 0 ) + CheckCellNoWinner ( *i, 4 ) + CheckCellNoWinner ( *i, 8 ) ) ;
			noWinner = noWinner & ( CheckCellNoWinner ( *i, 2 ) + CheckCellNoWinner ( *i, 4 ) + CheckCellNoWinner ( *i, 6 ) ) ;
		}

		return noWinner;
	}
}



/**
 * Create the default configuration struture of DDD
 */
DDD createInitial () {

	GDDD M0 = GDDD::one;

	for (size_t i = 0; i < NBCELL; ++i) {
		M0 = GDDD(i, EMPTY) ^  M0 ;
		std::stringstream cas;
		cas << "cell_" << i;
		DDD::varName(i,cas.str());
	}

	M0 = GDDD(STATE_SYSTEM_CELL, EMPTY) ^  M0 ;
	DDD::varName(STATE_SYSTEM_CELL,"State");
	return M0;
}

