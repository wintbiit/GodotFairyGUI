#include "g_tween.h"

#include <godot_cpp/core/math.hpp>

using namespace godot;

double GTween::ease_ratio(EaseType p_ease, double p_ratio, double p_amplitude, double p_period) const {
    p_ratio = CLAMP(p_ratio, 0.0, 1.0);
    const double _Pi = 3.14159265358979323846;
    const double _TwoPi = _Pi * 2.0;
    const double _PiOver2 = _Pi * 0.5;

    switch (p_ease) {
        case EASE_LINEAR_ENUM:
            return p_ratio;

        case EASE_SINE_IN_ENUM:
            return -Math::cos(p_ratio * _PiOver2) + 1.0;

        case EASE_SINE_OUT_ENUM:
            return Math::sin(p_ratio * _PiOver2);

        case EASE_SINE_IN_OUT_ENUM:
            return -0.5 * (Math::cos(_Pi * p_ratio) - 1.0);

        case EASE_QUAD_IN_ENUM:
            return p_ratio * p_ratio;

        case EASE_QUAD_OUT_ENUM:
            return -(p_ratio) * (p_ratio - 2.0);

        case EASE_QUAD_IN_OUT_ENUM:
            if (p_ratio < 0.5)
                return 2.0 * p_ratio * p_ratio;
            return 1.0 - Math::pow(-2.0 * p_ratio + 2.0, 2.0) / 2.0;

        case EASE_CUBIC_IN_ENUM:
            return p_ratio * p_ratio * p_ratio;

        case EASE_CUBIC_OUT_ENUM:
            return (p_ratio - 1.0) * (p_ratio - 1.0) * (p_ratio - 1.0) + 1.0;

        case EASE_CUBIC_IN_OUT_ENUM:
            if (p_ratio < 0.5)
                return 4.0 * p_ratio * p_ratio * p_ratio;
            return 1.0 - Math::pow(-2.0 * p_ratio + 2.0, 3.0) / 2.0;

        case EASE_QUART_IN_ENUM:
            return p_ratio * p_ratio * p_ratio * p_ratio;

        case EASE_QUART_OUT_ENUM:
            return 1.0 - Math::pow(1.0 - p_ratio, 4.0);

        case EASE_QUART_IN_OUT_ENUM:
            if (p_ratio < 0.5)
                return 8.0 * p_ratio * p_ratio * p_ratio * p_ratio;
            return 1.0 - Math::pow(-2.0 * p_ratio + 2.0, 4.0) / 2.0;

        case EASE_QUINT_IN_ENUM:
            return p_ratio * p_ratio * p_ratio * p_ratio * p_ratio;

        case EASE_QUINT_OUT_ENUM:
            return 1.0 - Math::pow(1.0 - p_ratio, 5.0);

        case EASE_QUINT_IN_OUT_ENUM:
            if (p_ratio < 0.5)
                return 16.0 * p_ratio * p_ratio * p_ratio * p_ratio * p_ratio;
            return 1.0 - Math::pow(-2.0 * p_ratio + 2.0, 5.0) / 2.0;

        case EASE_EXPO_IN_ENUM:
            return p_ratio == 0.0 ? 0.0 : Math::pow(2.0, 10.0 * (p_ratio - 1.0));

        case EASE_EXPO_OUT_ENUM:
            return p_ratio >= 1.0 ? 1.0 : 1.0 - Math::pow(2.0, -10.0 * p_ratio);

        case EASE_EXPO_IN_OUT_ENUM:
            if (p_ratio == 0.0) return 0.0;
            if (p_ratio >= 1.0) return 1.0;
            if (p_ratio < 0.5)
                return Math::pow(2.0, 10.0 * (2.0 * p_ratio - 1.0)) * 0.5;
            return (2.0 - Math::pow(2.0, -10.0 * (2.0 * p_ratio - 1.0))) * 0.5;

        case EASE_CIRC_IN_ENUM:
            return -(Math::sqrt(1.0 - p_ratio * p_ratio) - 1.0);

        case EASE_CIRC_OUT_ENUM:
            return Math::sqrt(1.0 - (p_ratio - 1.0) * (p_ratio - 1.0));

        case EASE_CIRC_IN_OUT_ENUM:
            if (p_ratio < 0.5)
                return -(Math::sqrt(1.0 - 4.0 * p_ratio * p_ratio) - 1.0) * 0.5;
            return (Math::sqrt(1.0 - 4.0 * (p_ratio - 1.0) * (p_ratio - 1.0)) + 1.0) * 0.5;

        case EASE_ELASTIC_IN_ENUM: {
            if (p_ratio == 0.0) return 0.0;
            if (p_ratio >= 1.0) return 1.0;
            double period = p_period > 0.0 ? p_period : 0.3;
            double s;
            double amplitude = MAX(1.0, p_amplitude);
            s = period / _TwoPi * Math::asin(1.0 / amplitude);
            p_ratio -= 1.0;
            return -(amplitude * Math::pow(2.0, 10.0 * p_ratio) * Math::sin((p_ratio - s) * _TwoPi / period));
        }

        case EASE_ELASTIC_OUT_ENUM: {
            if (p_ratio == 0.0) return 0.0;
            if (p_ratio >= 1.0) return 1.0;
            double period = p_period > 0.0 ? p_period : 0.3;
            double s;
            double amplitude = MAX(1.0, p_amplitude);
            s = period / _TwoPi * Math::asin(1.0 / amplitude);
            return amplitude * Math::pow(2.0, -10.0 * p_ratio) * Math::sin((p_ratio - s) * _TwoPi / period) + 1.0;
        }

        case EASE_ELASTIC_IN_OUT_ENUM: {
            if (p_ratio == 0.0) return 0.0;
            if (p_ratio >= 1.0) return 1.0;
            double period = p_period > 0.0 ? p_period : 0.3 * 1.5;
            double s;
            double amplitude = MAX(1.0, p_amplitude);
            s = period / _TwoPi * Math::asin(1.0 / amplitude);
            if (p_ratio < 0.5) {
                p_ratio -= 1.0;
                return -0.5 * (amplitude * Math::pow(2.0, 10.0 * p_ratio) * Math::sin((p_ratio - s) * _TwoPi / period));
            }
            p_ratio -= 1.0;
            return amplitude * Math::pow(2.0, -10.0 * p_ratio) * Math::sin((p_ratio - s) * _TwoPi / period) * 0.5 + 1.0;
        }

        case EASE_BACK_IN_ENUM:
            return p_ratio * p_ratio * ((p_amplitude + 1.0) * p_ratio - p_amplitude);

        case EASE_BACK_OUT_ENUM: {
            double r = p_ratio - 1.0;
            return r * r * ((p_amplitude + 1.0) * r + p_amplitude) + 1.0;
        }

        case EASE_BACK_IN_OUT_ENUM: {
            double amplitude = p_amplitude * 1.525;
            if (p_ratio < 0.5)
                return 0.5 * (p_ratio * 2.0 * p_ratio * 2.0 * ((amplitude + 1.0) * p_ratio * 2.0 - amplitude));
            double r = p_ratio * 2.0 - 2.0;
            return 0.5 * (r * r * ((amplitude + 1.0) * r + amplitude) + 2.0);
        }

        case EASE_BOUNCE_IN_ENUM:
            return bounce_ease_in(p_ratio);

        case EASE_BOUNCE_OUT_ENUM:
            return bounce_ease_out(p_ratio);

        case EASE_BOUNCE_IN_OUT_ENUM:
            return bounce_ease_in_out(p_ratio);

        case EASE_CUSTOM_ENUM:
        default:
            return p_ratio;
    }
}

double GTween::bounce_ease_out(double p_ratio) const {
    const double n1 = 7.5625;
    const double d1 = 2.75;

    if (p_ratio < 1.0 / d1) {
        return n1 * p_ratio * p_ratio;
    } else if (p_ratio < 2.0 / d1) {
        p_ratio -= 1.5 / d1;
        return n1 * p_ratio * p_ratio + 0.75;
    } else if (p_ratio < 2.5 / d1) {
        p_ratio -= 2.25 / d1;
        return n1 * p_ratio * p_ratio + 0.9375;
    } else {
        p_ratio -= 2.625 / d1;
        return n1 * p_ratio * p_ratio + 0.984375;
    }
}

double GTween::bounce_ease_in(double p_ratio) const {
    return 1.0 - bounce_ease_out(1.0 - p_ratio);
}

double GTween::bounce_ease_in_out(double p_ratio) const {
    if (p_ratio < 0.5) {
        return bounce_ease_in(p_ratio * 2.0) * 0.5;
    }
    return bounce_ease_out(p_ratio * 2.0 - 1.0) * 0.5 + 0.5;
}
