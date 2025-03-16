/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   forks.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mwojtcza <mwojtcza@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/11 14:48:35 by mwojtcza          #+#    #+#             */
/*   Updated: 2024/09/11 14:48:35 by mwojtcza         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/philo.h"

int	get_next_philosopher_id(t_philosopher *philo)
{
	t_simulation	*sim;

	sim = philo->sim;
	if (philo->id == sim->number_of_philosophers)
		return (0);
	return (philo->id);
}

int	get_previous_philosopher_id(t_philosopher *philo)
{
	t_simulation	*sim;

	sim = philo->sim;
	if (philo->id == 1)
		return (sim->number_of_philosophers - 1);
	return (philo->id - 2);
}

int	is_neighbor_hungrier(t_philosopher *philo, int next_id, int prev_id)
{
	t_simulation	*sim;
	long			t_meal_next;
	long			t_meal_prev;
	long			t_meal_cur;

	sim = philo->sim;
	pthread_mutex_lock(&sim->philosophers[next_id].meal_mutex);
	t_meal_next = cur_time_ms() - sim->philosophers[next_id].last_meal_time;
	pthread_mutex_unlock(&sim->philosophers[next_id].meal_mutex);
	pthread_mutex_lock(&sim->philosophers[prev_id].meal_mutex);
	t_meal_prev = cur_time_ms() - sim->philosophers[prev_id].last_meal_time;
	pthread_mutex_unlock(&sim->philosophers[prev_id].meal_mutex);
	pthread_mutex_lock(&philo->meal_mutex);
	t_meal_cur = cur_time_ms() - philo->last_meal_time;
	pthread_mutex_unlock(&philo->meal_mutex);
	return (t_meal_next > t_meal_cur || t_meal_prev > t_meal_cur);
}

void	lock_forks(t_philosopher *philo)
{
	if (philo->id % 2 == 0)
	{
		pthread_mutex_lock(philo->right_fork);
		print_action(philo, "has taken a fork");
		pthread_mutex_lock(philo->left_fork);
		print_action(philo, "has taken a fork");
	}
	else
	{
		pthread_mutex_lock(philo->left_fork);
		print_action(philo, "has taken a fork");
		pthread_mutex_lock(philo->right_fork);
		print_action(philo, "has taken a fork");
	}
}
// wt = wait_time

void	pick_up_forks(t_philosopher *philo)
{
	int			next_id;
	int			prev_id;
	//int			wt;

	next_id = get_next_philosopher_id(philo);
	prev_id = get_previous_philosopher_id(philo);
	//wt = 0;
	while (1)
	{
		if (is_neighbor_hungrier(philo, next_id, prev_id))
			usleep(420);
		else
		{
			lock_forks(philo);
			break ;
		}
	}
}
