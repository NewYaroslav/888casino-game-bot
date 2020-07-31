#ifndef CASIO_STRATEGY_HPP_INCLUDED
#define CASIO_STRATEGY_HPP_INCLUDED

#include <iostream>
#include <ctime>
#include <random>

namespace casino {

    /** \brief Стратегия
     * Для живой европейской рулетки.
     * Тут нужно учесть всевозможные ожидания времени ставки, смены дилера, учесть пропажу интернета, когда зависло, но потом вернулось или глюк трансляции.
     *
     * 1 смотрим какой цвет только что был или зеро. Если зеро, пропуск 6-20 спинов рандомно.
     *
     * 2 в зависимости от цвета, что выпало, красный, черный, зеро ставим на такой же цвет что и предыдущий мин ставку.
     * Если зеро, запоминаем, какая была предыдущая ставка, пропускаем рандомно от 6 до 20 пустых спинов и продолжаем по алгоритму. (красный 0.1)
     *
     * 3 если после пункта 2 получился выигрыш, то ставка та же на тот же цвет. Если получился проигрыш, то ставим на противоположный и удваиваем.
     * (красный, выигрыш 0.1 ставка  на красный 0.1) ( черный проигрыш, ставка на черный 0.2) (зеро проирали.)
     * Если зеро, запоминаем сумму ставки перед зеро, пропускаем рандомно от 6 до 20 пустых спинов, потом смотрим, чтобы выпало минимум 2 любых цвета подряд,
     * как нашли это триггер к ожиданию смены цвета и на следующий, после смены, с удвоенным объемом делаем ставку
     *
     * 4 если пункт 3 повторился четыре раза подряд, черное, красное, черное.
     * На 5 раз ставка не делается, пропускаем рандомно от 6 до 20 пустых спинов, потом смотрим, чтобы выпало минимум 2 любых цвета подряд,
     * как нашли это триггер к ожиданию смены цвета и на следующий, после смены, с удвоенным объемом делаем ставку.
     */
    class Strategy {
    public:

        /// Типы сигналов
        enum class TypesSignal {
            RED,
            BLACK,
            ZERO
        };

        class Bet {
        public:
            TypesSignal signal;
            int step = 0;
            Bet() {};

            std::string get_color() {
                if(signal == TypesSignal::RED) return std::string("RED");
                if(signal == TypesSignal::BLACK) return std::string("BLACK");
                if(signal == TypesSignal::ZERO) return std::string("ZERO");
                return std::string();
            }

            void clear() {
                step = 0;
            }
        };

    private:
        TypesSignal last_signal;
        int step = 0;
        int step_max = 0;

        int skip_step = 0;
        int skip_step_max = 0;
        bool skip = false;

        bool is_zero = false;
        int zero_bet_step = 0;

        int series_bet_step = 0;
        bool is_series = false;

        /** \brief Пропуск ставок
         * пропуск 6-20 спинов рандомно.
         */
        void skip_bet() {
            if(skip) return;
            std::mt19937 gen(std::time(nullptr));
            std::uniform_int_distribution<> distrib(6, 20);
            skip_step_max = distrib(gen);
            std::cout << "rnd skip: " << skip_step_max << std::endl;
            skip_step = 0;
            skip = true;
        };

        /** \brief Обработать пропуск ставок
         * \return Вернет true
         */
        bool check_skip() {
            if(skip) {
                ++skip_step;
                if(skip_step >= skip_step_max) {
                    skip = false;
                    skip_step = 0;
                    skip_step_max = 0;
                }
                return false;
            }
            return true;
        }

        void make_bet(const TypesSignal signal, Bet &new_bet) {
            new_bet.step = 0;
            /* Если ранее выпала серия сделок */
            if(is_series) {
                /* пропускаем zero */
                if(signal == TypesSignal::ZERO) {
                    last_signal = signal;
                    series_bet_step = 0;
                    return;
                };

                /* если это первый цвет */
                if(series_bet_step == 0) {
                    /* запоминили цвет, увеличили счетчик */
                    last_signal = signal;
                    ++series_bet_step;
                    return;
                }

                /* если это второй цвет */
                if(series_bet_step == 1 && last_signal == signal) {
                    /* цвет совпадает два раза подряд */
                    ++series_bet_step;
                    return;
                } else
                /* если это третий цвет, противоположный */
                if(series_bet_step == 2 && last_signal != signal) {
                    /* цвет сменился,
                     * после смены, с удвоенным объемом делаем ставку*/
                    ++step;
                    last_signal = signal;
                    new_bet.step = step;
                    new_bet.signal = last_signal;
                    /* выходим из обработки сигнала серии */
                    series_bet_step = 0;
                    is_series = false;
                    return;
                } else {
                    last_signal = signal;
                    series_bet_step = 0;
                }
                return;
            } else
            /* если ранее выпадал zero */
            if(is_zero) {
                /* пропускаем zero */
                if(signal == TypesSignal::ZERO) {
                    last_signal = signal;
                    zero_bet_step = 0;
                    return;
                };

                /* если это первый цвет */
                if(zero_bet_step == 0) {
                    /* запоминили цвет, увеличили счетчик */
                    last_signal = signal;
                    ++zero_bet_step;
                    return;
                }

                /* если это второй цвет */
                if(zero_bet_step == 1 && last_signal == signal) {
                    /* цвет совпадает два раза подряд */
                    ++zero_bet_step;
                } else
                /* если это третий цвет, противоположный */
                if(zero_bet_step == 2 && last_signal != signal) {
                    /* цвет сменился,
                     * после смены, с удвоенным объемом делаем ставку*/
                    last_signal = signal;
                    ++step;
                    last_signal = signal;
                    new_bet.step = step;
                    new_bet.signal = last_signal;
                    /* выходим из обраьотки сигнала zero */
                    zero_bet_step = 0;
                    is_zero = false;
                } else {
                    last_signal = signal;
                    zero_bet_step = 0;
                }
                return;
            }

            /* первая ставка */
            if(step == 0) {
                if(signal == TypesSignal::ZERO) {
                    /* пропуск 6-20 спинов рандомно. */
                    skip_bet();
                    return;
                }
                ++step;
                last_signal = signal;
                new_bet.step = step;
                new_bet.signal = last_signal;
            } else
            /* вторая ставка */
            if(step >= 1) {
                if(signal == last_signal) {
                    /* если после пункта 2 получился выигрыш, то ставка та же на тот же цвет. */
                    step = 1;
                    new_bet.step = step;
                    new_bet.signal = last_signal;
                    series_bet_step = 0;
                    return;
                } else {
                    /* зеро - проирали. */
                    if(signal == TypesSignal::ZERO) {
                        /* Если зеро, запоминаем сумму ставки перед зеро,
                         * пропускаем рандомно от 6 до 20 пустых спинов, потом смотрим,
                         * чтобы выпало минимум 2 любых цвета подряд,
                         * как нашли это триггер к ожиданию смены цвета и на следующий,
                         * после смены, с удвоенным объемом делаем ставку
                         */
                        new_bet.step = 0;
                        new_bet.signal = TypesSignal::ZERO;
                        zero_bet_step = 0;
                        series_bet_step = 0;
                        is_zero = true;
                        return;
                    }
                    ++series_bet_step;
                    /* На 5 раз ставка не делается, пропускаем рандомно от 6 до 20 пустых спинов */
                    if(series_bet_step == 5) {
                        series_bet_step = 0;
                        is_series = true;
                        /* пропуск 6-20 спинов рандомно. */
                        skip_bet();
                        return;
                    }
                    /* Если получился проигрыш, то ставим на противоположный и удваиваем. */
                    ++step;
                    last_signal = signal;
                    new_bet.step = step;
                    new_bet.signal = last_signal;
                    return;
                }
            }
        }

    public:

        /** \brief Обновить состояние стратегии
         * \param signal Состояние сигнала (итог  последней ставки)
         * \param new_bet Новая сделка
         */
        void update(const TypesSignal signal, Bet &new_bet) {
            new_bet.step = 0;
            new_bet.signal = signal;
            switch(signal) {
            case TypesSignal::RED: {
                    /* пропускаем заход, если нужно */
                    if(check_skip()) {
                        make_bet(signal, new_bet);
                    }
                }
                break;
            case TypesSignal::BLACK: {
                    /* пропускаем заход, если нужно */
                    if(check_skip()) {
                        make_bet(signal, new_bet);
                    }
                }
                break;
            case TypesSignal::ZERO: {
                    /* пропускаем заход, если нужно */
                    if(check_skip()) {
                        make_bet(signal, new_bet);
                    }
                }
                break;
            default:
                break;
            };
        }

        /** \brief Очистить состояние стратегии
         */
        void clear() {
            step = 0;
            step_max = 0;
            skip_step = 0;
            skip_step_max = 0;
            skip = false;
            is_zero = false;
            zero_bet_step = 0;
            series_bet_step = 0;
            is_series = false;
        }
    };

};


#endif // CASIO_STRATEGY_HPP_INCLUDED
