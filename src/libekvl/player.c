#include "player.h"

#include "ekio.h"	/* EKVL_C_* */
#include "eknote.h"
#include "ektype.h"
#include "vchannel.h"

#ifdef __GNUC__
static inline
#else
static
#endif
EkErrFlag play_note(EkSetup *settings, EkInstr *samp, EkBlock *block, int *commandInt)
{
  int	ch;
  EkNote	*note= EKBLOCK_FIELD(*block, note[
				EKSETUP_FIELD(*settings, blkPos)]);

	EKSETUP_FIELD(*settings, ttempo)= 0;

	dbuff_blockpos(EKSETUP_FIELD(*settings, blkPos));
	for (ch= 0; ch < EKSETUP_FIELD(*settings, tracks); ch++)
	{
	    /* flush this evolving string if we need a bug trace */
#ifdef SLOW_DISPLAY
	    if (ch < 4)	/* display too small for more than 4 */
		dbuff_channel(note[ch].pitch, note[ch].instNum,
				note[ch].fx, note[ch].fxdat);
#else
	    if (ch < 4)
		dbuff_channel(EKNOTE_FIELD(note[ch], text));
#endif
	    set_vchan(ch, &(note[ch]),samp,settings);
	}

	/* output block info. to display... */
	dbuff_flush();

	/* ...and samples to audio: */
  return flush_vchans(settings, commandInt);
}

EkErrFlag play_song(EkSong *song, EkSetup *settings, int *commandInt)
{
  EkBlock	*currBlk;
  int		command= EKVL_C_NONE,
		samNum= 0;
  Flag		stop= FL_FALSE;
  EkErrFlag	error= EK_ERR_NONE;

	set_verbosity(EKSETUP_FIELD(*settings, verbose));
	set_menu(0);

	disp_songtitle(eksong_title(song));
	disp_sampdata(0, EKSONG_FIELD(*song, instr));
	EKSETUP_FIELD(*settings, songType)=
					EKSONG_FIELD(*song, songType);
	init_vchans(EKSONG_FIELD(*song, maxtracks), settings);

	EKSETUP_FIELD(*settings, seqPos)=
				EKSETUP_FIELD(*settings, seqSkip);
	EKSETUP_FIELD(*settings, ptempo)=
				EKSONG_FIELD(*song, tempo);
	EKSETUP_FIELD(*settings, stempo)=
				EKSONG_FIELD(*song, fineTempo);
	EKSETUP_FIELD(*settings, flags1)=
				EKSONG_FIELD(*song, flags1);
	EKSETUP_FIELD(*settings, flags2)=
				EKSONG_FIELD(*song, flags2);

	if (EKSETUP_FIELD(*settings, volume) == 0)
	    EKSETUP_FIELD(*settings, volume)=
					EKSONG_FIELD(*song, mstrvol);
	if (EKSETUP_FIELD(*settings, transpose) == 0)
	    EKSETUP_FIELD(*settings, transpose)=
					EKSONG_FIELD(*song, transpose);

	while ( !stop && (EKSETUP_FIELD(*settings, seqPos)
				< EKSONG_FIELD(*song, playseqs)) )
	{
	    EKSETUP_FIELD(*settings, blkNum)=
			EKSONG_FIELD(*song, playseq[
				EKSETUP_FIELD(*settings, seqPos)]);

	    currBlk= &EKSONG_FIELD(*song, block[
				EKSETUP_FIELD(*settings, blkNum)]);
	    EKSETUP_FIELD(*settings, tracks)=
				EKBLOCK_FIELD(*currBlk, tracks);
	    EKSETUP_FIELD(*settings, outrate)= reset_vchans(settings);

	    disp_blockhdr(EKSETUP_FIELD(*settings, seqPos),
			EKSONG_FIELD(*song, playseqs),
			EKSETUP_FIELD(*settings, blkNum),
			EKSONG_FIELD(*song, blocks),
			EKBLOCK_FIELD(*currBlk, name));

	    EKSETUP_FIELD(*settings, blkPos= 0);
	    EKSETUP_FIELD(*settings, blkLen)=
				EKBLOCK_FIELD(*currBlk, lines);
	    while ( !stop && (EKSETUP_FIELD(*settings, blkPos)
				< EKSETUP_FIELD(*settings, blkLen)) )
	    {
		if ( (error= play_note(
				settings, EKSONG_FIELD(*song, instr),
				currBlk, &command)) != EK_ERR_NONE )
		{
		    command= EKVL_C_QUIT;
		    stop= FL_TRUE;
		}
		else
		{
		    switch (command)
		    {
		    case EKVL_C_QUIT:
			stop= FL_TRUE;
			break;
		    case EKVL_C_NEXT:
			stop= FL_TRUE;
			break;
		    case EKVL_C_FWD:
			EKSETUP_FIELD(*settings, blkPos)=
				EKSETUP_FIELD(*settings, blkLen);
			break;
		    case EKVL_C_REW:
			if (EKSETUP_FIELD(*settings, blkPos) == 0)
			    stop= FL_TRUE;
			else
			{
			    EKSETUP_FIELD(*settings, blkPos)= 0;
			    stop= FL_FALSE;
			}
			break;
		    case EKVL_C_NEWPOS:
			stop= FL_TRUE;
			break;
		    case EKVL_C_SAMNXT:
		    case EKVL_C_SAMPRV:
			samNum+= (command == EKVL_C_SAMNXT)
				?   ( samNum < (EKSONG_FIELD(*song,
						lastinstr)-1) )
				: -(int) (samNum > 0);
			disp_sampdata(samNum,
				&EKSONG_FIELD(*song, instr[samNum]));
			stop= FL_FALSE;
			break;
		    case EKVL_C_SAMFRST:
		    case EKVL_C_SAMLAST:
			samNum= (command == EKVL_C_SAMFRST)?  0
				: EKSONG_FIELD(*song, lastinstr)-1;
			disp_sampdata(samNum,
				&EKSONG_FIELD(*song, instr[samNum]));
			stop= FL_FALSE;
			break;
		    }

		    if (command != EKVL_C_REW)
			EKSETUP_FIELD(*settings, blkPos)++;
		}
	    }
		
	    if (command == EKVL_C_REW)
	    {
		if (EKSETUP_FIELD(*settings, seqPos) > 0)
		    EKSETUP_FIELD(*settings, seqPos)--;
		stop= FL_FALSE;
	    }
	    else if ( (command != EKVL_C_NEWPOS)
			&& (command != EKVL_C_NEXT)
			&& (command != EKVL_C_QUIT) )
	    {
		EKSETUP_FIELD(*settings, seqPos)++;
		stop= FL_FALSE;
	    }
	}

	close_vchans();
	if (commandInt) *commandInt= command;
  return error;
}
