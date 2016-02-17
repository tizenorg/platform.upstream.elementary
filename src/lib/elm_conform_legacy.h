/**
 * Add a new conformant widget to the given parent Elementary
 * (container) object.
 *
 * @param parent The parent object.
 * @return A new conformant widget handle or @c NULL, on errors.
 *
 * This function inserts a new conformant widget on the canvas.
 *
 * @ingroup Conformant
 */
EAPI Evas_Object                 *elm_conformant_add(Evas_Object *parent);

// TIZEN_ONLY(20160218): Improve launching performance.
/**
 * @internal
 *
 * Set the precreated object.
 *
 * @param obj The conformant object
 *
 * @ingroup Conformant
 * @see elm_conformant_precreated_object_get()
 * @since Tizen 2.4
 */
EAPI void                         elm_conformant_precreated_object_set(Evas_Object *obj);

/**
 * @internal
 *
 * Get the precreated object.
 *
 * @return The precreated conformant object
 *
 * @ingroup Conformant
 * @see elm_conformant_precreated_object_set()
 * @since Tizen 2.4
 */
EAPI Evas_Object                 *elm_conformant_precreated_object_get(void);
//
