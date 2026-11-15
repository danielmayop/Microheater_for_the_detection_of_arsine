function [T0_cool, tau_off0] = estimar_inicial_cool(t_cool_rel, T_cool, T_amb)
% ESTIMAR_INICIAL_COOL Estima T0_cool y un valor inicial de tau_off para
% el ajuste del modelo de enfriamiento.
%
%   Modelo: T(t) = T_amb + (T0-T_amb)*exp(-t/tau_off)
%
%   Entradas:
%       t_cool_rel - tiempo relativo del tramo de enfriamiento (empieza en 0)
%       T_cool     - temperatura del tramo de enfriamiento
%       T_amb      - temperatura ambiente
%
%   Salidas:
%       T0_cool  - temperatura inicial del enfriamiento
%       tau_off0 - valor inicial de tau_off [s]

    % Temperatura inicial del enfriamiento (promedio de primeros puntos)
    N0 = min(10, length(T_cool));
    T0_cool = mean(T_cool(1:N0));

    % Buscamos el tiempo donde queda el 36.8% de la diferencia inicial
    % respecto al ambiente.
    T368 = T_amb + 0.368*(T0_cool - T_amb);
    [~, indice_tau_off] = min(abs(T_cool - T368));
    tau_off0 = t_cool_rel(indice_tau_off);
end
