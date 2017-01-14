/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   chrparse.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tboos <marvin@42.fr>                       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2016/05/11 14:29:42 by tboos             #+#    #+#             */
/*   Updated: 2017/01/12 18:11:58 by maxpetit         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "minishell.h"

void			ft_flushend(t_stream *stream)
{
	size_t		size;

	ft_repeat_termcaps(1, "cd", stream);
	if (stream->search)
		ft_flushsearch(stream);
	else if (stream->command && stream->command[0])
	{
		size = ft_strlen(stream->command + stream->pos);
		ft_putstr_fd(stream->command + stream->pos, SFD);
		stream->pos += size;
	}
	if (stream->pos && stream->command[stream->pos - 1] != '\n'
			&& ((ssize_t *)(stream->buf))[0] != DEL
			&& ft_checknewline(stream, stream->pos) == 0)
		ft_repeat_termcaps(1, "do", stream);
	ft_erase(stream);
}

void			ft_flush(t_stream *stream)
{
	size_t		pos;

	pos = stream->pos;
	ft_flushend(stream);
	while (stream->pos != pos)
		ft_mvleft(stream);
}

void			ft_append(t_stream *stream)
{
	size_t		pos;
	size_t		len;
	char		*kill;

	len = ft_strlen(stream->buf);
	if ((kill = stream->command))
	{
		pos = stream->pos;
		if (!(stream->command = ft_strnew(ft_strlen(stream->command) + len))
			&& (stream->state = -2) && ft_freegiveone((void **)&kill))
			return ;
		ft_strncpy(stream->command, kill, pos);
		ft_strcpy(stream->command + pos, stream->buf);
		ft_strcpy(stream->command + pos + len, kill + pos);
		ft_freegiveone((void **)&kill);
	}
	else if (!(stream->command = ft_strdup(stream->buf)))
		stream->state = -2;
	ft_push_history(stream, stream->config, 1);
	ft_flush(stream);
	while (len--)
		ft_mvright(stream);
}

/*
**               PARSE HEXA FOR KEY MAPPING
** printf("\nbuf   = %lx\n", ((ssize_t *)(stream->buf))[0]);
** printf("\nmatch = %lx\n", match[i]);
*/

static int		ft_chrmatch(t_stream *stream)
{
	static ssize_t		match[] = {CLF, SUP, CHT, DEL, LEF, RIG, UPP, DOW, CLEF,
			CRIG, CUPP, CDOW, END, HOM, CRS, ESC, ALTS, CTRLL, VISV, VISD,
			VISY, VISP, NUL};
	int					i;

	i = 0;
	while (match[i])
	{
		if (((ssize_t *)(stream->buf))[0] == match[i])
			return (i);
		i++;
	}
	if (ft_isprint(stream->buf[0]) || ft_isspace(stream->buf[0]))
		return (-1);
	return (-2);
}

/*
** Run the appropriate function for a key touched.
*/

int				ft_chrparse(t_stream *stream)
{
	int					match;
	static void			(*ftab[])(t_stream *) = {&ft_sup, &ft_autocomp, &ft_del,
			&ft_left, &ft_right, &ft_up, &ft_down, &ft_ctrlleft, &ft_ctrlright,
			&ft_ctrlup, &ft_ctrldown, &ft_goend, &ft_gohome, &ft_searchengine,
			&ft_searchengineend, &ft_syntax_color, &ft_clear, &ft_visselect,
			&ft_viscut, &ft_viscopy, &ft_vispaste};

	if (COMP_STATE == 2 && ((ssize_t*)(stream->buf))[0] == CLF)
		ft_end_autocomp(stream);
	else if (ft_is_same_autocomp(stream))
		(*ftab[1])(stream);
	else
	{
		if ((match = ft_chrmatch(stream)) == -1)
		{
			ft_strchrsed(stream->buf, '\t', ' ');
			stream->search ? ft_sappend(stream) : ft_append(stream);
			if (ft_strchr(stream->buf, '\n') && !ft_pastereturn(stream))
				match = 0;
		}
		else if (match > 0)
			(*ftab[match - 1])(stream);
		if (!match)
			return (0);
	}
	return (1);
}
