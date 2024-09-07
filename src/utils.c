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

long long	current_timestamp(void)
{
    struct timeval	tv;
	
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void	print_status(t_philosopher *philo, char *status)
{
    pthread_mutex_lock(&philo->data->print_lock);
    printf("%lld %d %s\n", current_timestamp() - philo->data->start_time, philo->id, status);
    pthread_mutex_unlock(&philo->data->print_lock);
}

int ft_atoi(const char *nptr)
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

void	handle_single_philosopher(t_data *data)
{
    printf("%lld 0 is thinking\n", current_timestamp() - data->start_time);
    usleep(data->time_to_die * 1000);
    printf("%lld 0 died\n", current_timestamp() - data->start_time);
}

// void cleanup(t_data *data, t_philosopher *philos)
// {
//     int i = 0;
//     while (i < data->number_of_philosophers)
//     {
//         pthread_join(philos[i].thread, NULL);
//         pthread_mutex_destroy(&data->forks[i]);
//         i++;
//     }
//     pthread_mutex_destroy(&data->print_lock);
//     free(data->forks);
//     free(philos);
// }

void cleanup(t_data *data, t_philosopher *philos) {
    for (int i = 0; i < data->number_of_philosophers; i++) {
        pthread_join(philos[i].thread, NULL);  // Ensure all threads have finished
    }
    // Then proceed to destroy mutexes and free memory
    for (int i = 0; i < data->number_of_philosophers; i++) {
        pthread_mutex_destroy(&data->forks[i]);
    }
    pthread_mutex_destroy(&data->print_lock);
    free(data->forks);
    free(philos);
}