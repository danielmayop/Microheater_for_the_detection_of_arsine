function graficar_heat(t_heat, T_heat, T_pred_heat, ax)
% GRAFICAR_HEAT Grafica temperatura medida vs modelo durante el calentamiento.
%
%   GRAFICAR_HEAT(t_heat, T_heat, T_pred_heat) abre una figura nueva.
%   GRAFICAR_HEAT(t_heat, T_heat, T_pred_heat, ax) dibuja sobre el eje
%   'ax' (por ejemplo, el handle devuelto por subplot(...)).

    if nargin < 4 || isempty(ax)
        figure;
        ax = axes;
    end

    plot(ax, t_heat, T_heat, 'b', 'LineWidth', 1);
    hold(ax, 'on');
    plot(ax, t_heat, T_pred_heat, 'r--', 'LineWidth', 2);
    grid(ax, 'on');
    xlabel(ax, 'Tiempo [s]');
    ylabel(ax, 'Temperatura [°C]');
    title(ax, 'Identificacion del modelo de calentamiento');
    legend(ax, 'Temperatura medida', 'Modelo de primer orden');
    hold(ax, 'off');
end