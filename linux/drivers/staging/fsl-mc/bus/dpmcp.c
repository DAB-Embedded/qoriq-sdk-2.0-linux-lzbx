/* Copyright 2013-2016 Freescale Semiconductor Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * * Neither the name of the above-listed copyright holders nor the
 * names of any contributors may be used to endorse or promote products
 * derived from this software without specific prior written permission.
 *
 *
 * ALTERNATIVELY, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") as published by the Free Software
 * Foundation, either version 2 of that License or (at your option) any
 * later version.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "../include/mc-sys.h"
#include "../include/mc-cmd.h"
#include "dpmcp.h"
#include "dpmcp-cmd.h"

int dpmcp_open(struct fsl_mc_io *mc_io,
	       uint32_t cmd_flags,
	       int dpmcp_id,
	       uint16_t *token)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_OPEN,
					  cmd_flags,
					  0);
	cmd.params[0] |= mc_enc(0, 32, dpmcp_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*token = get_mc_cmd_hdr_token(cmd.header);

	return err;
}

int dpmcp_close(struct fsl_mc_io *mc_io,
		uint32_t cmd_flags,
		uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_CLOSE,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmcp_create(struct fsl_mc_io *mc_io,
		 uint16_t dprc_token,
		 uint32_t cmd_flags,
		 const struct dpmcp_cfg *cfg,
		 uint32_t *object_id)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_CREATE,
					cmd_flags, dprc_token);
	cmd.params[0] |= mc_enc(0, 32, cfg->portal_id);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*object_id = get_mc_cmd_create_object_id(&cmd);

	return 0;
}

int dpmcp_destroy(struct fsl_mc_io *mc_io,
		  uint16_t dprc_token,
		  uint32_t cmd_flags,
		  uint32_t object_id)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_DESTROY,
					cmd_flags,
					dprc_token);
	/* set object id to destroy */
	set_mc_cmd_destroy_object_id(&cmd, object_id);
	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmcp_reset(struct fsl_mc_io *mc_io,
		uint32_t cmd_flags,
		uint16_t token)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_RESET,
					  cmd_flags,
					  token);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmcp_set_irq(struct fsl_mc_io *mc_io,
		  uint32_t cmd_flags,
		  uint16_t token,
		  uint8_t irq_index,
		  struct dpmcp_irq_cfg	*irq_cfg)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_SET_IRQ,
					  cmd_flags,
					  token);
	cmd.params[0] |= mc_enc(0, 8, irq_index);
	cmd.params[0] |= mc_enc(32, 32, irq_cfg->val);
	cmd.params[1] |= mc_enc(0, 64, irq_cfg->paddr);
	cmd.params[2] |= mc_enc(0, 32, irq_cfg->user_irq_id);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmcp_get_irq(struct fsl_mc_io *mc_io,
		  uint32_t cmd_flags,
		  uint16_t token,
		  uint8_t irq_index,
		  int *type,
		  struct dpmcp_irq_cfg	*irq_cfg)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_GET_IRQ,
					  cmd_flags,
					  token);
	cmd.params[0] |= mc_enc(32, 8, irq_index);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	irq_cfg->val = (uint32_t)mc_dec(cmd.params[0], 0, 32);
	irq_cfg->paddr = (uint64_t)mc_dec(cmd.params[1], 0, 64);
	irq_cfg->user_irq_id = (int)mc_dec(cmd.params[2], 0, 32);
	*type = (int)mc_dec(cmd.params[2], 32, 32);
	return 0;
}

int dpmcp_set_irq_enable(struct fsl_mc_io *mc_io,
			 uint32_t cmd_flags,
			 uint16_t token,
			 uint8_t irq_index,
			 uint8_t en)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_SET_IRQ_ENABLE,
					  cmd_flags,
					  token);
	cmd.params[0] |= mc_enc(0, 8, en);
	cmd.params[0] |= mc_enc(32, 8, irq_index);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmcp_get_irq_enable(struct fsl_mc_io *mc_io,
			 uint32_t cmd_flags,
			 uint16_t token,
			 uint8_t irq_index,
			 uint8_t *en)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_GET_IRQ_ENABLE,
					  cmd_flags,
					  token);
	cmd.params[0] |= mc_enc(32, 8, irq_index);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*en = (uint8_t)mc_dec(cmd.params[0], 0, 8);
	return 0;
}

int dpmcp_set_irq_mask(struct fsl_mc_io *mc_io,
		       uint32_t cmd_flags,
		       uint16_t token,
		       uint8_t irq_index,
		       uint32_t mask)
{
	struct mc_command cmd = { 0 };

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_SET_IRQ_MASK,
					  cmd_flags,
					  token);
	cmd.params[0] |= mc_enc(0, 32, mask);
	cmd.params[0] |= mc_enc(32, 8, irq_index);

	/* send command to mc*/
	return mc_send_command(mc_io, &cmd);
}

int dpmcp_get_irq_mask(struct fsl_mc_io *mc_io,
		       uint32_t cmd_flags,
		       uint16_t token,
		       uint8_t irq_index,
		       uint32_t *mask)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_GET_IRQ_MASK,
					  cmd_flags,
					  token);
	cmd.params[0] |= mc_enc(32, 8, irq_index);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*mask = (uint32_t)mc_dec(cmd.params[0], 0, 32);
	return 0;
}

int dpmcp_get_irq_status(struct fsl_mc_io *mc_io,
			 uint32_t cmd_flags,
			 uint16_t token,
			 uint8_t irq_index,
			 uint32_t *status)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_GET_IRQ_STATUS,
					  cmd_flags,
					  token);
	cmd.params[0] |= mc_enc(32, 8, irq_index);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	*status = (uint32_t)mc_dec(cmd.params[0], 0, 32);
	return 0;
}

int dpmcp_get_attributes(struct fsl_mc_io *mc_io,
			 uint32_t cmd_flags,
			 uint16_t token,
			 struct dpmcp_attr *attr)
{
	struct mc_command cmd = { 0 };
	int err;

	/* prepare command */
	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_GET_ATTR,
					  cmd_flags,
					  token);

	/* send command to mc*/
	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	/* retrieve response parameters */
	attr->id = (int)mc_dec(cmd.params[0], 32, 32);

	return 0;
}

int dpmcp_get_api_version(struct fsl_mc_io *mc_io,
			   uint32_t cmd_flags,
			   uint16_t *major_ver,
			   uint16_t *minor_ver)
{
	struct mc_command cmd = { 0 };
	int err;

	cmd.header = mc_encode_cmd_header(DPMCP_CMDID_GET_API_VERSION,
					cmd_flags,
					0);

	err = mc_send_command(mc_io, &cmd);
	if (err)
		return err;

	get_mc_cmd_object_api_ver(&cmd, major_ver, minor_ver);

	return 0;
}
