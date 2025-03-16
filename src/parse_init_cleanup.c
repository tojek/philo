/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_init_cleanup.c                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mwojtcza <mwojtcza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 15:19:32 by mwojtcza          #+#    #+#             */
/*   Updated: 2024/09/11 15:19:32 by mwojtcza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

void	parse_arguments(int argc, char **argv, t_simulation *sim)
{
	if (argc < 5 || argc > 6)
		print_error("Invalid number of arguments");
	sim->number_of_philosophers = ft_atoi(argv[1]);
	sim->time_to_die = ft_atoi(argv[2]);
	sim->time_to_eat = ft_atoi(argv[3]);
	sim->time_to_sleep = ft_atoi(argv[4]);
	if (argc == 6)
	{
		sim->number_of_times_each_philosopher_must_eat = ft_atoi(argv[5]);
		sim->is_optional_arg_present = 1;
	}
	else
		sim->is_optional_arg_present = 0;
	if (sim->number_of_philosophers <= 0 || sim->time_to_die <= 0
		|| sim->time_to_eat <= 0 || sim->time_to_sleep <= 0)
	{
		print_error("Arguments must be positive integers");
	}
	if (sim->is_optional_arg_present
		&& sim->number_of_times_each_philosopher_must_eat <= 0)
	{
		print_error("Optional argument must be a positive integer");
	}
}

void	init_forks(t_simulation *sim)
{
	int	i;

	i = 0;
	sim->forks = malloc(sizeof(pthread_mutex_t) * sim->number_of_philosophers);
	if (!sim->forks)
		print_error("Failed to allocate memory for forks");
	while (i < sim->number_of_philosophers)
	{
		if (pthread_mutex_init(&sim->forks[i], NULL) != 0)
			print_error("Failed to initialize fork mutex");
		i++;
	}
	if (pthread_mutex_init(&sim->log_mutex, NULL) != 0)
		print_error("Failed to initialize log mutex");
	if (pthread_mutex_init(&sim->death_mutex, NULL) != 0)
		print_error("Failed to initialize death mutex");
	sim->death_flag = 0;
	sim->finished_philosophers = 0;
}

void	init_philosophers(t_simulation *sim)
{
	int	i;

	i = 0;
	sim->philosophers = malloc(sizeof(t_philosopher)
			* sim->number_of_philosophers);
	if (!sim->philosophers)
		print_error("Failed to allocate memory for philosophers");
	while (i < sim->number_of_philosophers)
	{
		sim->philosophers[i].id = i + 1;
		sim->philosophers[i].times_eaten = 0;
		sim->philosophers[i].last_meal_time = cur_time_ms();
		sim->philosophers[i].left_fork = &sim->forks[i];
		sim->philosophers[i].right_fork = &sim->forks[(i + 1)
			% sim->number_of_philosophers];
		sim->philosophers[i].sim = sim;
		pthread_mutex_init(&sim->philosophers[i].meal_mutex, NULL);
		i++;
	}
}

void	cleanup_forks(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_philosophers)
	{
		pthread_mutex_destroy(&sim->forks[i]);
		pthread_mutex_destroy(&sim->philosophers[i].meal_mutex);
		i++;
	}
	free(sim->forks);
	free(sim->philosophers);
	pthread_mutex_destroy(&sim->log_mutex);
	pthread_mutex_destroy(&sim->death_mutex);
}
