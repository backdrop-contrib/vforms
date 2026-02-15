# Node Form Variants (vforms)

Node Form Variants lets you create and manage multiple **edit-form layouts** (“variants”) per content type. Each variant can include/exclude fields, change ordering, and optionally organize fields into groups.

**Machine name:** `vforms`  
**Title (project):** Node Form Variants

## Features

- Create any number of form variants per node type (bundle).
- Reorder fields and “extra fields” (form elements provided by modules).
- Show/hide fields per variant.
- Create simple groups and drag fields into them.
- Control group fieldset behavior per group:
  - **open** (collapsible, initially open)
  - **closed** (collapsible, initially closed)
  - **locked open** (non-collapsible)
- Stores configuration in Backdrop’s config system (exportable/importable).
- Provides a **Form variant edit link** Views field

## Requirements

- Backdrop CMS 1.x
- Core jQuery UI dialog library is used for the “floaty dialog” variant editor where applicable.

## Installation

- Install this module using the official Backdrop CMS instructions at
  https://docs.backdropcms.org/documentation/extend-with-modules.
- Enable the module at:  
   **Administration → Functionality → List modules**

## Permissions

Grant permissions as needed at:  
**Administration → People → Permissions**

Look for permissions under the `vforms` module section.

## Configuration / Where to find it

For each content type:

- Go to: **Administration → Structure → Content types**
- Click **Form variants** for the type.

Variant edit path pattern:

`admin/structure/types/manage/%type/vforms/%variant/edit`

## Configuration storage (database impact)

vforms does **not** create custom database tables.

Variants are stored as Backdrop configuration objects, typically:

- `vforms.node.<bundle>`

Example:
- `vforms.node.article`

These config objects can be exported/imported via **Configuration Management**.

## Theming notes (dialog)

The floating editor dialog uses Backdrop’s AJAX dialog (jQuery UI). The dialog “chrome” is jQuery UI; the dialog *content* is rendered with whichever theme Backdrop applies to that route (site theme vs admin theme).

If your site is configured to “Use the administration theme when editing or creating content”, the dialog content may inherit the admin theme even on non-`/admin` paths.

## Project status

This module is in early (alpha) stage. APIs and UI may change.

## Current Maintainers

- [ericfoy](https://github.com/ericfoy)

## Credits

- This module was created for Backdrop by [ericfoy](https://github.com/ericfoy)
- Sponsored by [Perideo LLC](https://perideo.com).

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
