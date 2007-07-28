/***************************************************************************
 *            job.h
 *
 *  dim jan 22 10:40:26 2006
 *  Copyright  2006  Rouquier Philippe
 *  brasero-app@wanadoo.fr
 ***************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef JOB_H
#define JOB_H

#include <glib.h>
#include <glib-object.h>

#include "burn-basics.h"
#include "burn-track.h"

G_BEGIN_DECLS

#define BRASERO_TYPE_JOB         (brasero_job_get_type ())
#define BRASERO_JOB(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), BRASERO_TYPE_JOB, BraseroJob))
#define BRASERO_JOB_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), BRASERO_TYPE_JOB, BraseroJobClass))
#define BRASERO_IS_JOB(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), BRASERO_TYPE_JOB))
#define BRASERO_IS_JOB_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), BRASERO_TYPE_JOB))
#define BRASERO_JOB_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), BRASERO_TYPE_JOB, BraseroJobClass))

typedef enum {
	BRASERO_JOB_ACTION_NONE		= 0,
	BRASERO_JOB_ACTION_SIZE,
	BRASERO_JOB_ACTION_IMAGE,
	BRASERO_JOB_ACTION_RECORD,
	BRASERO_JOB_ACTION_ERASE,
	BRASERO_JOB_ACTION_CHECKSUM
} BraseroJobAction;

typedef struct {
	GObject parent;
} BraseroJob;

typedef struct {
	GObjectClass parent_class;

	/**
	 * Virtual functions to implement in each object deriving from
	 * BraseroJob. start is the only one which is compulsory.
	 */

	BraseroBurnResult	(*init)			(BraseroJob *job,
							 GError **error);

	/**
	 * returns 	OK if we should carry on
	 * 		NOT_RUNNING if it completed successfully but task
	 *		shouldn't go any further
	 * 		ERROR otherwise
	 */
	BraseroBurnResult	(*start)		(BraseroJob *job,
							 GError **error);

	BraseroBurnResult	(*clock_tick)		(BraseroJob *job);

	BraseroBurnResult	(*stop)			(BraseroJob *job,
							 GError **error);

	/**
	 * you should not connect to this signal. It's used internally to 
	 * "autoconfigure" the backend when an error occurs
	 */
	BraseroBurnResult	(*error)		(BraseroJob *job,
							 BraseroBurnError error);
} BraseroJobClass;

GType brasero_job_get_type ();

/**
 * These functions are to be used to get information for running jobs.
 * They are only available when a job is running.
 */

BraseroBurnResult
brasero_job_get_action (BraseroJob *job, BraseroJobAction *action);

BraseroBurnResult
brasero_job_get_flags (BraseroJob *job, BraseroBurnFlag *flags);

BraseroBurnResult
brasero_job_get_fd_in (BraseroJob *job, int *fd_in);

BraseroBurnResult
brasero_job_get_tracks (BraseroJob *job, GSList **tracks);

BraseroBurnResult
brasero_job_get_done_tracks (BraseroJob *job, GSList **tracks);

BraseroBurnResult
brasero_job_get_current_track (BraseroJob *job, BraseroTrack **track);

BraseroBurnResult
brasero_job_get_input_type (BraseroJob *job, BraseroTrackType *type);

BraseroBurnResult
brasero_job_get_audio_title (BraseroJob *job, gchar **album);

BraseroBurnResult
brasero_job_get_data_label (BraseroJob *job, gchar **label);

BraseroBurnResult
brasero_job_get_session_size (BraseroJob *job, guint64 *blocks, guint64 *size);

/**
 * Used to get information of the destination media
 */

BraseroBurnResult
brasero_job_get_device (BraseroJob *job, gchar **device);

BraseroBurnResult
brasero_job_get_media (BraseroJob *job, BraseroMedia *media);

BraseroBurnResult
brasero_job_get_last_session_address (BraseroJob *job, guint64 *address);

BraseroBurnResult
brasero_job_get_next_writable_address (BraseroJob *job, guint64 *address);

BraseroBurnResult
brasero_job_get_rate (BraseroJob *job, guint64 *rate);

BraseroBurnResult
brasero_job_get_speed (BraseroJob *self, guint *speed);

BraseroBurnResult
brasero_job_get_max_rate (BraseroJob *job, guint64 *rate);

BraseroBurnResult
brasero_job_get_max_speed (BraseroJob *job, guint *speed);

/**
 * necessary for objects imaging either to another or to a file
 */

BraseroBurnResult
brasero_job_get_output_type (BraseroJob *job, BraseroTrackType *type);

BraseroBurnResult
brasero_job_get_fd_out (BraseroJob *job, int *fd_out);

BraseroBurnResult
brasero_job_get_image_output (BraseroJob *job,
			      gchar **image,
			      gchar **toc);
BraseroBurnResult
brasero_job_get_audio_output (BraseroJob *job,
			      gchar **output);

/**
 * get a temporary file that will be deleted once the session is finished
 */
 
BraseroBurnResult
brasero_job_get_tmp_file (BraseroJob *job,
			  gchar **output,
			  GError **error);

BraseroBurnResult
brasero_job_get_tmp_dir (BraseroJob *job,
			 gchar **path,
			 GError **error);

/**
 * Used to give job results and tell when a job has finished
 */

BraseroBurnResult
brasero_job_finished (BraseroJob *job,
		      BraseroTrack *track);

BraseroBurnResult
brasero_job_error (BraseroJob *job,
		   GError *error);

/**
 * Used to start progress reporting and starts an internal timer to keep track
 * of remaining time among other things
 */

BraseroBurnResult
brasero_job_start_progress (BraseroJob *job,
			    gboolean force);

/**
 * task progress report: you can use only some of these functions
 */

BraseroBurnResult
brasero_job_set_rate (BraseroJob *job,
		      gint64 rate);
BraseroBurnResult
brasero_job_set_written (BraseroJob *job,
			 gint64 written);
BraseroBurnResult
brasero_job_set_progress (BraseroJob *job,
			  gdouble progress);
BraseroBurnResult
brasero_job_set_current_action (BraseroJob *job,
				BraseroBurnAction action,
				const gchar *string,
				gboolean force);
BraseroBurnResult
brasero_job_get_current_action (BraseroJob *job,
				BraseroBurnAction *action);
BraseroBurnResult
brasero_job_set_current_track_size (BraseroJob *job,
				    guint64 block_size,
				    guint64 sectors,
				    gint64 size);

BraseroBurnResult
brasero_job_add_wrong_checksum (BraseroJob *job,
				const gchar *path);

/**
 * Used to tell it's (or not) dangerous to interrupt this job
 */

void
brasero_job_set_dangerous (BraseroJob *job, gboolean value);

/**
 * This is for apps with a jerky current rate (like cdrdao)
 */

BraseroBurnResult
brasero_job_set_use_average_rate (BraseroJob *job,
				  gboolean value);

/**
 * logging facilities
 */

void
brasero_job_log_message (BraseroJob *job,
			 const gchar *location,
			 const gchar *format,
			 ...);

#define BRASERO_JOB_LOG(job, message, ...) 			\
{								\
	gchar *format;						\
	format = g_strdup_printf ("%s %s",			\
				  G_OBJECT_TYPE_NAME (job),	\
				  message);			\
	brasero_job_log_message (BRASERO_JOB (job),		\
				 G_STRLOC,			\
				 format,		 	\
				 ##__VA_ARGS__);		\
	g_free (format);					\
}
#define BRASERO_JOB_LOG_ARG(job, message, ...)			\
{								\
	gchar *format;						\
	format = g_strdup_printf ("\t%s",			\
				  (gchar*) message);		\
	brasero_job_log_message (BRASERO_JOB (job),		\
				 G_STRLOC,			\
				 format,			\
				 ##__VA_ARGS__);		\
	g_free (format);					\
}

#define BRASERO_JOB_NOT_SUPPORTED(job) 					\
	{								\
		BRASERO_JOB_LOG (job, "unsupported operation");		\
		return BRASERO_BURN_NOT_SUPPORTED;			\
	}

#define BRASERO_JOB_NOT_READY(job)						\
	{									\
		BRASERO_JOB_LOG (job, "not ready to operate");	\
		return BRASERO_BURN_NOT_READY;					\
	}


G_END_DECLS

#endif /* JOB_H */
