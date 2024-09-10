/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mwojtcza <mwojtcza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/07/17 12:55:25 by mwojtcza          #+#    #+#             */
/*   Updated: 2024/07/17 12:55:25 by mwojtcza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

long	current_time_in_ms(void)
{
	struct timeval	time;

	gettimeofday(&time, NULL);
	return ((time.tv_sec * 1000) + (time.tv_usec / 1000));
}

int	ft_atoi(const char *nptr)
{
	int	i;
	int	sign;
	int	res;

	i = 0;
	sign = 1;
	res = 0;
	while (nptr[i] == 32 || (nptr[i] >= 9 && nptr[i] <= 13))
		i++;
	if (nptr[i] == '+' || nptr[i] == '-')
	{
		if (nptr[i] == '-')
			sign = -1;
		i++;
	}
	while (nptr[i] && nptr[i] >= 48 && nptr[i] <= 57)
	{
		res *= 10;
		res += (nptr[i] - 48);
		i++;
	}
	return (res *= sign);
}

void	print_error(char *message)
{
	printf("Error: %s\n", message);
	exit(EXIT_FAILURE);
}

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
}

void cleanup_forks(t_simulation *sim)
{
    for (int i = 0; i < sim->number_of_philosophers; i++)
    {
        pthread_mutex_destroy(&sim->forks[i]);
        pthread_mutex_destroy(&sim->philosophers[i].meal_mutex);  // Destroy per-philosopher meal mutex
    }

    free(sim->forks);  // Clean up fork memory
    free(sim->philosophers);  // Clean up philosophers memory

    pthread_mutex_destroy(&sim->log_mutex);
    pthread_mutex_destroy(&sim->death_mutex);
}


void init_philosophers(t_simulation *sim)
{
    sim->philosophers = malloc(sizeof(t_philosopher) * sim->number_of_philosophers);
    if (!sim->philosophers)
        print_error("Failed to allocate memory for philosophers");

    for (int i = 0; i < sim->number_of_philosophers; i++)
    {
        sim->philosophers[i].id = i + 1;  // Philosopher IDs are 1-based
        sim->philosophers[i].times_eaten = 0;
        sim->philosophers[i].last_meal_time = current_time_in_ms();
        sim->philosophers[i].left_fork = &sim->forks[i];
        sim->philosophers[i].right_fork = &sim->forks[(i + 1) % sim->number_of_philosophers];
        sim->philosophers[i].sim = sim;

        // Initialize the per-philosopher meal mutex
        pthread_mutex_init(&sim->philosophers[i].meal_mutex, NULL);
		//usleep(10);
    }
}


void handle_single_philosopher(t_philosopher *philo)
{
    print_action(philo, "is thinking");
    pthread_mutex_lock(philo->left_fork);
    print_action(philo, "has taken a fork");
    usleep(philo->sim->time_to_die * 1000);
    print_action(philo, "died");
    pthread_mutex_unlock(philo->left_fork);
}


// Helper function to print actions
void print_action(t_philosopher *philo, const char *action)
{
    long timestamp;

	pthread_mutex_lock(&philo->sim->log_mutex);
    if (philo->sim->death_flag == 0)  // Only log if no philosopher has died
    {
        timestamp = current_time_in_ms() - philo->sim->start_time;
        printf("%ld %d %s\n", timestamp, philo->id, action);
    }
	pthread_mutex_unlock(&philo->sim->log_mutex);
}