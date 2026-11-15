function graficar_cool(t_cool, T_cool, T_pred_cool, ax)
% GRAFICAR_COOL Grafica temperatura medida vs modelo durante el enfriamiento.
%
%   GRAFICAR_COOL(t_cool, T_cool, T_pred_cool) abre una figura nueva.
%   GRAFICAR_COOL(t_cool, T_cool, T_pred_cool, ax) dibuja sobre el eje
%   'ax' (por ejemplo, el handle devuelto por subplot(...)).

    if nargin < 4 || isempty(ax)
        figure;
        ax = axes;
    end

    plot(ax, t_cool, T_cool, 'b', 'LineWidth', 1);
    hold(ax, 'on');
    plot(ax, t_cool, T_pred_cool, 'r--', 'LineWidth', 2);
    grid(ax, 'on');
    xlabel(ax, 'Tiempo [s]');
    ylabel(ax, 'Temperatura [°C]');
    title(ax, 'Identificacion del modelo de enfriamiento');
    legend(ax, 'Temperatura medida', 'Modelo de enfriamiento');
    hold(ax, 'off');
end