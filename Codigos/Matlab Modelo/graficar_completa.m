function graficar_completa(t, T, T_pred_total, ax)
% GRAFICAR_COMPLETA Grafica la medición completa vs el modelo identificado
% a lo largo de todo el experimento.
%
%   GRAFICAR_COMPLETA(t, T, T_pred_total) abre una figura nueva.
%   GRAFICAR_COMPLETA(t, T, T_pred_total, ax) dibuja sobre el eje 'ax'
%   (por ejemplo, el handle devuelto por subplot(...)).

    if nargin < 4 || isempty(ax)
        figure;
        ax = axes;
    end

    plot(ax, t, T, 'b', 'LineWidth', 1);
    hold(ax, 'on');
    plot(ax, t, T_pred_total, 'r--', 'LineWidth', 2);
    grid(ax, 'on');
    xlabel(ax, 'Tiempo [s]');
    ylabel(ax, 'Temperatura [°C]');
    title(ax, 'Microcalentador: medicion vs modelo');
    legend(ax, 'Temperatura medida', 'Modelo identificado');
    hold(ax, 'off');
end