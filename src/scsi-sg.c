/***************************************************************************
 *            burn-sg.c
 *
 *  Wed Oct 18 14:39:28 2006
 *  Copyright  2006  Rouquier Philippe
 *  <bonfire-app@wanadoo.fr>
 ****************************************************************************/

/*
 * Brasero is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Brasero is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor Boston, MA 02110-1301,  USA
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include <scsi/scsi.h>
#include <scsi/sg.h>

#include "scsi-command.h"
#include "burn-debug.h"
#include "scsi-utils.h"
#include "scsi-error.h"
#include "scsi-sense-data.h"

struct _BraseroDeviceHandle {
	int fd;
};

struct _BraseroScsiCmd {
	uchar cmd [BRASERO_SCSI_CMD_MAX_LEN];
	BraseroDeviceHandle *handle;

	const BraseroScsiCmdInfo *info;
};
typedef struct _BraseroScsiCmd BraseroScsiCmd;

#define BRASERO_SCSI_CMD_OPCODE_OFF			0
#define BRASERO_SCSI_CMD_SET_OPCODE(command)		(command->cmd [BRASERO_SCSI_CMD_OPCODE_OFF] = command->info->opcode)

#define OPEN_FLAGS			O_RDWR /*|O_EXCL */|O_NONBLOCK

/**
 * This is to send a command
 */

static void
brasero_sg_command_setup (struct sg_io_hdr *transport,
			  uchar *sense_data,
			  BraseroScsiCmd *cmd,
			  uchar *buffer,
			  int size)
{
	memset (sense_data, 0, BRASERO_SENSE_DATA_SIZE);
	memset (transport, 0, sizeof (struct sg_io_hdr));
	
	transport->interface_id = 'S';				/* mandatory */
//	transport->flags = SG_FLAG_LUN_INHIBIT|SG_FLAG_DIRECT_IO;
	transport->cmdp = cmd->cmd;
	transport->cmd_len = cmd->info->size;
	transport->dxferp = buffer;
	transport->dxfer_len = size;

	/* where to output the scsi sense buffer */
	transport->sbp = sense_data;
	transport->mx_sb_len = BRASERO_SENSE_DATA_SIZE;

	if (cmd->info->direction & BRASERO_SCSI_READ)
		transport->dxfer_direction = SG_DXFER_FROM_DEV;
	else if (cmd->info->direction & BRASERO_SCSI_WRITE)
		transport->dxfer_direction = SG_DXFER_TO_DEV;
}

BraseroScsiResult
brasero_scsi_command_issue_sync (gpointer command,
				 gpointer buffer,
				 int size,
				 BraseroScsiErrCode *error)
{
	uchar sense_buffer [BRASERO_SENSE_DATA_SIZE];
	struct sg_io_hdr transport;
	BraseroScsiResult res;
	BraseroScsiCmd *cmd;

	cmd = command;
	brasero_sg_command_setup (&transport,
				  sense_buffer,
				  cmd,
				  buffer,
				  size);

	/* NOTE on SG_IO: only for TEST UNIT READY, REQUEST/MODE SENSE, INQUIRY,
	 * READ CAPACITY, READ BUFFER, READ and LOG SENSE are allowed with it */
	res = ioctl (cmd->handle->fd, SG_IO, &transport);
	if (res) {
		BRASERO_SCSI_SET_ERRCODE (error, BRASERO_SCSI_ERRNO);
		return BRASERO_SCSI_FAILURE;
	}

	if ((transport.info & SG_INFO_OK_MASK) == SG_INFO_OK)
		return BRASERO_SCSI_OK;

	if ((transport.masked_status & CHECK_CONDITION) && transport.sb_len_wr)
		return brasero_sense_data_process (sense_buffer, error);

	return BRASERO_SCSI_FAILURE;
}

gpointer
brasero_scsi_command_new (const BraseroScsiCmdInfo *info,
			  BraseroDeviceHandle *handle) 
{
	BraseroScsiCmd *cmd;

	/* make sure we can set the flags of the descriptor */

	/* allocate the command */
	cmd = g_new0 (BraseroScsiCmd, 1);
	cmd->info = info;
	cmd->handle = handle;

	BRASERO_SCSI_CMD_SET_OPCODE (cmd);
	return cmd;
}

BraseroScsiResult
brasero_scsi_command_free (gpointer cmd)
{
	g_free (cmd);
	return BRASERO_SCSI_OK;
}

/**
 * This is to open a device
 */

BraseroDeviceHandle *
brasero_device_handle_open (const gchar *path,
			    gboolean exclusive,
			    BraseroScsiErrCode *code)
{
	int fd;
	int flags = OPEN_FLAGS;
	BraseroDeviceHandle *handle;

	if (exclusive)
		flags |= O_EXCL;

	fd = open (path, flags);
	if (fd < 0) {
		if (code) {
			if (errno == EAGAIN
			||  errno == EWOULDBLOCK
			||  errno == EBUSY)
				*code = BRASERO_SCSI_NOT_READY;
			else
				*code = BRASERO_SCSI_ERRNO;
		}

		return NULL;
	}

	handle = g_new (BraseroDeviceHandle, 1);
	handle->fd = fd;

	return handle;
}

void
brasero_device_handle_close (BraseroDeviceHandle *handle)
{
	close (handle->fd);
	g_free (handle);
}

