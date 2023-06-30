/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   microshell.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: llord <llord@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/06/19 13:35:56 by llord             #+#    #+#             */
/*   Updated: 2023/06/30 11:34:09 by llord            ###   ########.fr       */
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
	int	fd_pipe[2]; //											pipe exit and entry
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

//		write_error("Running command : ", av[0]); //	0======== DEBUG ========0

		if (i != 0) //													if a command needs to be ran
		{
			if (strcmp(av[0], "cd") == 0) //									if first arg == "cd"
			{
				if (i != 2)
					write_error("error: cd: bad arguments", NULL); //					if not exactly one arg; ERROR
				else if (chdir(av[1]) != 0)
					write_error("error: cd: cannot change directory to ", av[1]); //	if unable to use chdir; ERROR
			}
			else if (!av[i] || !strcmp(av[i], ";")) //						if breakpoint is NULL or ";"
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
				pipe(fd_pipe); //														init pipes
				if (fork() == 0) //														fork; if child
				{
					dup2(fd_pipe[1], STDOUT_FILENO); //											sets output to pipe entry
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
		i += 1;
		j += i; //													skips breakpoint at av[i]
	}
	close(fd_in);
	return (0);
}



/*

smaller (note) version


unistd
stdio
string
sys/wait


in = 1
out = 0

fork_exec(av, ev, fd_io)
{
	if (fork)
	{
		dup2(fd_io[out], stdout)
		dup2(fd_io[in], stdin)
		close(fd_io[out])
		close(fd_io[in])

		execve(av[0], av, ev)
		error
		exit(1)
	}
	else
	{
		close (fd_io[in])
		while (waitpid(-1, NULL, WUNTRACED));
		close (fd_io[out])
	}
}

main (ac, av, ev)
{
	int fd_pipe[2]
	int fd_io[2]
	int i = 1
	int loop = 1

	fd_io[out] = dup(stdout)
	fd_io[in] = dup(stdin)

	while (loop < ac)
	{
		av = &(av[i])
		i = 0

		while (!breakpoint)
			i++

		if (cd)
		{
			if (i != 2)
				error
			if (chdir(av[0]))
				error
		}
		else if (NULL or ;)
		{
			av[i] = NULL

			fd_io[out] = dup(stdout)
			fork_exec(av, ev, fd_io)
			fd_io[in] = dup(stdin)
		}
		else if (|)
		{
			pipe(fd_pipe)
			av[i] = NULL

			fd_io[out] = fd_pipe[in]
			fork_exec(av, ev, fd_io)
			fd_io[in] = fd_pipe[out]
		}
		i += 1
		loop += i
	}

	close(fd_io[out])
	close(fd_io[in])
	exit (0)
}
*/