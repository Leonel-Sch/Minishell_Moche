/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: lscheupl <lscheupl@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/12/06 17:41:00 by leonel            #+#    #+#             */
/*   Updated: 2024/12/12 20:27:12 by lscheupl         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../inc/minishell.h"
#include <stdio.h>

void	ft_clean_path(char **tab_path)
{
	int	i;
	int	j;

	i = 0;
	j = ft_strlen(tab_path[0]) - 5;
	while (i < j)
	{
		tab_path[0][i] = tab_path[0][i + 5];
		i++;
	}
}

char	*ft_convert_pos_to_string(char *input, int start, int end)
{
	char	*res;
	int		i;
	int		j;

	res = ft_calloc((end - start + 1), sizeof(char));
	i = 0;
	j = start;
	while (i < end - start)
	{
		res[i++] = input[j++];
	}
	res[end - start] = '\0';
	return (res);
}

char	*ft_expander(char *input, int start, int end)
{
	return (ft_convert_pos_to_string(input, start, end));
}
char	**ft_isolate_path(t_ms *ms)
{
	char	*path;
	char	**tab_path;
	int		i;

	i = 0;
	while (ms->envp[i])
	{
		if (ms->envp[i][0] == 'P' && ms->envp[i][1] == 'A'
			&& ms->envp[i][2] == 'T' && ms->envp[i][3] == 'H')
			path = ft_strdup(ms->envp[i]);
		i++;
	}
	tab_path = ft_split(path, ':');
	ft_clean_path(tab_path);
	free(path);
	return (tab_path);
}

char	*ft_check_access(char **tab_path, char *cmd)
{
	char	test[255];
	char	*res;
	int		i;

	i = 0;
	res = NULL;
	if (access(cmd, X_OK) == 0)
		return (res = ft_strdup(cmd));
	while (tab_path[i])
	{
		ft_strlcpy(test, tab_path[i], 256);
		ft_strlcat(test, "/", 256);
		ft_strlcat(test, cmd, 256);
		if (access(test, X_OK) == 0)
			break ;
		ft_bzero(test, 256);
		i++;
	}
	if (access(test, X_OK) == 0)
	{
		res = malloc(sizeof(char) * (ft_strlen(test) + 1));
		ft_strlcpy(res, test, (ft_strlen(test) + 1));
	}
	printf("%s\n", res);
	return (res);
}
char	*ft_isolate_first_word(char *expanded)
{
	int		i;
	char	*cmd;

	i = 0;
	while (expanded[i] != ' ' && expanded[i] != '\0')
	{
		i++;
	}
	cmd = malloc(sizeof(char) * i + 2);
	i = 0;
	while (expanded[i] != ' ' && expanded[i] != '\0')
	{
		cmd[i] = expanded[i];
		i++;
	}
	cmd[i] = '\0';
	return (cmd);
}

char	*ft_find_path(char *expanded, t_ms *ms, t_node_cmd *root)
{
	char		**tab_path;
	char		*bin;
	char		*cmd;

	bin = NULL;
	root = &ms->ast->cmd;
	tab_path = ft_isolate_path(ms);
	bin = ft_check_access(tab_path, root->args[0]);
	ft_free_split(tab_path);
	return (bin);
}

void	ft_execve(char *path, char **tab_arg, char **envp)
{
	execve(path, tab_arg, envp);
	printf("%s\n", path);
	return (ft_putendl_fd("minishell : command not found", 2));
}

void	exec_cmd(char *input, t_ast *root, t_ms *ms)
{
	int			i;
	t_node_cmd	*node;
	char		**tab_arg;
	char		*path;
	pid_t		pid;

	

	node = &(root->cmd);
	node->expanded = ft_expander(input, node->start, node->end);
	node->args = ft_split(node->expanded, ' ');
	path = ft_find_path(node->expanded, ms, node);
	tab_arg = ft_split(node->expanded, 32);
	pid = fork();
	if (pid == -1)
		return ;
	if (pid == 0)
	{
		ft_execve(path, tab_arg, ms->envp);
		exit(EXIT_SUCCESS);
	}
	waitpid(pid, NULL, 0);
	free(path);
	ft_free_split(tab_arg);
}

void	exec_pip(char *input, t_ast *root, t_ms *ms)
{
	int			i;
	t_node_pip	*node;

	i = 0;
	node = &(root->pip);
	printf("pip\n");
	printf("%s\n __", input);
	node->pip_redir = malloc((sizeof (int**)));
	while (i < node->pip_len)
    {
		pipe(node->pip_redir[i]);
		if (i != 0)
			dup2(node->pip_redir[i][0], STDIN_FILENO);
		dup2(node->pip_redir[i][1], STDOUT_FILENO);
		close(node->pip_redir[i][0]);
		close(node->pip_redir[i][1]);
        exec_general(input, node->piped[i], ms);
		i++;
    }
}

void	exec_grp(char *input, t_ast *root, t_ms *ms)
{
	int			i;
	t_node_grp	*node;

	i = 0;
	node = &(root->grp);
	printf("grp\n");
	//ft_expander;
	exec_general(input, node->next, ms);
}

void	exec_logic(char *input, t_ast *root, t_ms *ms)
{
	int				i;
	t_node_logic	*node;

	i = 0;
	node = &(root->logic);
	printf("logic\n");
	exec_general(input, node->left, ms);
	//if (exec == 1)
	exec_general(input, node->right, ms);
}
void	exec_general(char *input, t_ast *root, t_ms *ms)
{
	t_node_type node_type;

	node_type = root->type;
	if (node_type == E_CMD)
		exec_cmd(input, root, ms);
	else if (node_type == E_LOGIC)
		exec_logic(input, root, ms);
	else if (node_type == E_PIP)
		exec_pip(input, root, ms);
	else
		exec_grp(input, root, ms);
}