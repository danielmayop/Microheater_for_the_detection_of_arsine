function [t_seg, T_seg, t_seg_rel] = extraer_segmento(t, T, idx, t_ref)
% EXTRAER_SEGMENTO Extrae un tramo de la serie temporal según una máscara lógica.
%
%   [t_seg, T_seg, t_seg_rel] = EXTRAER_SEGMENTO(t, T, idx, t_ref)
%
%   Entradas:
%       t     - vector de tiempo completo
%       T     - vector de temperatura completo
%       idx   - vector lógico que selecciona las muestras del segmento
%       t_ref - instante de referencia para calcular el tiempo relativo
%               (t_on para calentamiento, t_off para enfriamiento)
%
%   Salidas:
%       t_seg     - tiempo absoluto del segmento
%       T_seg     - temperatura del segmento
%       t_seg_rel - tiempo relativo a t_ref (empieza en 0)

    t_seg = t(idx);
    T_seg = T(idx);
    t_seg_rel = t_seg - t_ref;
end
