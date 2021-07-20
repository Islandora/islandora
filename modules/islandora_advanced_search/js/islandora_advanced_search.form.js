//# sourceURL=modules/contrib/islandora/modules/islandora_advanced_search/js/islandora-advanced-search.form.js
/**
 * @file
 * Handles Ajax submission / updating form action on url change, etc.
 */
(function ($, Drupal, drupalSettings) {

  // Gets current parameters minus ones provided by the form.
  function getParams(query_parameter, recurse_parameter) {
    const url_search_params = new URLSearchParams(window.location.search);
    const params = Object.fromEntries(url_search_params.entries());
    // Remove Advanced Search Query Parameters.
    const param_match = "query\\[\\d+\\]\\[.+\\]".replace("query", query_parameter);
    const param_regex = new RegExp(param_match, "g");
    for (const param in params) {
      if (param.match(param_regex)) {
        delete params[param];
      }
    }
    // Remove Recurse parameter.
    delete params[recurse_parameter];
    // Remove the page if set as submitting the form should always take
    // the user to the first page (facets do the same).
    delete params["page"];
    return params;
  }

  // Groups form inputs by search term.
  function getTerms(inputs) {
    const input_regex = /terms\[(?<index>\d+)\]\[(?<component>.*)\]/;
    const terms = [];
    for (const input in inputs) {
      const name = inputs[input].name;
      const value = inputs[input].value;
      const found = name.match(input_regex);
      if (found) {
        const index = parseInt(found.groups.index);
        const component = found.groups.component;
        if (typeof terms[index] !== 'object') {
          terms[index] = {};
        }
        terms[index][component] = value;
      }
    }
    return terms;
  }

  // Checks if the form user has set recursive to true in the form.
  function getRecurse(inputs) {
    for (const input in inputs) {
      const name = inputs[input].name;
      const value = inputs[input].value;
      if (name == "recursive" && value == "1") {
        return true;
      }
    }
    return false;
  }

  function url(inputs, settings) {
    const terms = getTerms(inputs);
    const recurse = getRecurse(inputs);
    const params = getParams(settings.query_parameter, settings.recurse_parameter);
    for (const index in terms) {
      const term = terms[index];
      // Do not include terms with no value.
      if (term.value.length != 0) {
        for (const component in term) {
          const value = term[component];
          const param = "query[index][component]"
            .replace("query", settings.query_parameter)
            .replace("index", index)
            .replace("component", settings.mapping[component]);
          params[param] = value;
        }
      }
    }
    if (recurse) {
      params[settings.recurse_parameter] = '1';
    }
    return window.location.href.split("?")[0] + "?" + $.param(params);
  }

  Drupal.behaviors.islandora_advanced_search_form = {
    attach: function (context, settings) {
      if (settings.islandora_advanced_search_form.id !== 'undefined') {
        const $form = $('form#' + settings.islandora_advanced_search_form.id).once();
        if ($form.length > 0) {
          window.addEventListener("pushstate", function (e) {
            $form.attr('action', window.location.pathname + window.location.search);
          });
          window.addEventListener("popstate", function (e) {
            if (e.state != null) {
              $form.attr('action', window.location.pathname + window.location.search);
            }
          });
          // Prevent form submission and push state instead.
          //
          // Logic server side / client side should match to generate the 
          // appropriate URL with javascript enabled or disable.
          //
          // If a route is set for the view display that this form is derived
          // from, and we are not on the same page as that route, rely on the
          // normal submit which will redirect to the appropriate page. 
          if (!settings.islandora_advanced_search_form.redirect) {
            $form.submit(function (e) {
              e.preventDefault();
              e.stopPropagation();
              const inputs = $form.serializeArray();
              const href = url(inputs, settings.islandora_advanced_search_form);
              window.history.pushState(null, document.title, href);
            });
          }
          // Reset should trigger refresh of AJAX Blocks / Views.
          $form.find('input[data-drupal-selector = "edit-reset"]').mousedown(function (e) {
            const inputs = [];
            const href = url(inputs, settings.islandora_advanced_search_form);
            window.history.pushState(null, document.title, href);
          });
        }
      }
    }
  };
})(jQuery, Drupal, drupalSettings);
