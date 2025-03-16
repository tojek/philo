/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   supervisor.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mwojtcza <mwojtcza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 14:46:39 by mwojtcza          #+#    #+#             */
/*   Updated: 2024/09/11 14:46:39 by mwojtcza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

int	check_philosopher_death(t_simulation *sim, int i)
{
	long	time_from_meal;

	pthread_mutex_lock(&sim->philosophers[i].meal_mutex);
	time_from_meal = cur_time_ms() - sim->philosophers[i].last_meal_time;
	if (time_from_meal > sim->time_to_die)
	{
		pthread_mutex_lock(&sim->death_mutex);
		print_action(&sim->philosophers[i], "died");
		sim->death_flag = 1;
		pthread_mutex_unlock(&sim->death_mutex);
		pthread_mutex_unlock(&sim->philosophers[i].meal_mutex);
		return (1);
	}
	pthread_mutex_unlock(&sim->philosophers[i].meal_mutex);
	return (0);
}

int	check_philosopher_meals(t_simulation *sim)
{
	int	i;
	int	finished_philosophers;

	finished_philosophers = 0;
	i = 0;
	while (i < sim->number_of_philosophers)
	{
		pthread_mutex_lock(&sim->philosophers[i].meal_mutex);
		if (sim->is_optional_arg_present && sim->philosophers[i].times_eaten
			>= sim->number_of_times_each_philosopher_must_eat)
		{
			finished_philosophers++;
			sim->finished_philosophers = finished_philosophers;
		}
		pthread_mutex_unlock(&sim->philosophers[i].meal_mutex);
		i++;
	}
	return (sim->finished_philosophers == sim->number_of_philosophers);
}

void	check_philosopher_status(t_simulation *sim)
{
	int	i;

	i = 0;
	while (i < sim->number_of_philosophers)
	{
		if (check_philosopher_death(sim, i))
			return ;
		i++;
	}
}

void	*supervisor_routine(void *arg)
{
	t_simulation	*sim;

	sim = (t_simulation *)arg;
	while (1)
	{
		pthread_mutex_lock(&sim->death_mutex);
		if (sim->death_flag)
		{
			pthread_mutex_unlock(&sim->death_mutex);
			break ;
		}
		pthread_mutex_unlock(&sim->death_mutex);
		check_philosopher_status(sim);
		if (check_philosopher_meals(sim))
		{
			pthread_mutex_lock(&sim->death_mutex);
			sim->death_flag = 1;
			pthread_mutex_unlock(&sim->death_mutex);
			printf("All philosophers have eaten enough times. Exiting...\n");
			return (NULL);
		}
	}
	return (NULL);
}

void	handle_single_philosopher(t_philosopher *philo)
{
	print_action(philo, "is thinking");
	pthread_mutex_lock(philo->left_fork);
	print_action(philo, "has taken a fork");
	usleep(philo->sim->time_to_die * 1000);
	print_action(philo, "died");
	pthread_mutex_unlock(philo->left_fork);
}
