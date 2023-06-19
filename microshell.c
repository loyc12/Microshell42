/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llord <llord@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 13:35:56 by llord             #+#    #+#             */
/*   Updated: 2023/06/19 13:55:01 by llord            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/*
	I stole this code from pasqualerossi on git (https://github.com/pasqualerossi/42-School-Exam-Rank-04.git/)
	I kept the general structure but tweaked it heavily, and also added comments to figure out wtf was happening

	I think I found an error, where the executable would only do the first command, and let the shell do the remainder
	Maybe this error was my refactoring's fault tho. Fixed it by changing the main while()'s conditions and adding a loop incrementer

	Not sure this passes the exam as is, but it's at least functional
*/

#include <string.h> //		strcmp
#include <unistd.h> //		write, dup, dup2, close, execve, chdir and fork
#include <sys/wait.h> //	waitpid

//prints errors to STDERR
int	write_error(char *str, char *arg)
{
	while (str && *str)
		write(2, str++, 1);
	if (arg)
		while (*arg)
			write(2, arg++, 1);
	write(2, "\n", 1);
	return (1);
}

//tries to execute a given command
int	ft_exe(char **av, int i, int fd_in, char **ev)
{

	av[i] = NULL; //											voids the breakpoint
	dup2(fd_in, STDIN_FILENO); //								set input to fd_in (STDIN or pipe exit)
	close(fd_in);
	execve(av[0], av, ev); //									tries to execute the command
	return (write_error("error: cannot execute ", av[0])); //	if we get there the command failed; ERROR
}

//microshell itself
int	main(int ac, char **av, char **ev)
{
	int	fd_pipe[2]; //												pipe exit and entry
	int	fd_in; //												input fd (STDIN or pipe exit)
	int	i; //													arg incrementer
	int j; //													loop incrementer

	i = 1; //													starts incrementers at 1 to skip the executable's name
	j = 1;
	fd_in = dup(STDIN_FILENO); //								sets default input to STDIN

	while (j < ac) //											while there's content left in av
	{
		av = &av[i]; //													sets av[0] to after executable name or last breakpoint
		i = 0; //														resets arg incrementer

		while (av[i] && strcmp(av[i], ";") && strcmp(av[i], "|")) //	counts args until next breakpoint
			i++;
		if (i != 0) //													if a command needs to be ran
		{
			if (strcmp(av[0], "cd") == 0) //									if first arg == "cd"
			{
				if (i != 2)
					write_error("error: cd: bad arguments", NULL); //					if not exactly one arg; ERROR
				else if (chdir(av[1]) != 0)
					write_error("error: cd: cannot change directory to ", av[1]); //	if unable to use chdir; ERROR
			}
			else if ((!av[i] || !strcmp(av[i], ";"))) //						if breakpoint is NULL or ";"
			{
				if (fork() == 0) //														fork; if child
				{
					if (ft_exe(av, i, fd_in, ev)) //											exec in fork
						return (1);
				}
				else //																	fork; if parent
				{
					close(fd_in);
					while (waitpid(-1, NULL, WUNTRACED) != -1) // 								waits for child
						;
					fd_in = dup(STDIN_FILENO); //												sets fd_in back to default (STDIN)
				}
			}
			else if (!strcmp(av[i], "|")) //									if breakpoint is "|"
			{
				pipe(fd_pipe); //															init pipes
				if (fork() == 0) //														fork; if child
				{
					dup2(fd_pipe[1], STDOUT_FILENO); //												sets output to pipe entry
					close(fd_pipe[0]);
					close(fd_pipe[1]);
					if (ft_exe(av, i, fd_in, ev)) //											exec in fork
						return (1);
				}
				else //																	fork; if parent
				{
					close(fd_pipe[1]);
					close(fd_in);
					fd_in = fd_pipe[0]; //															sets fd_in to pipe exit
				}
			}
		}
		j += i + 1; //													skips breakpoint at av[i]
	}
	close(fd_in);
	return (0);
}
